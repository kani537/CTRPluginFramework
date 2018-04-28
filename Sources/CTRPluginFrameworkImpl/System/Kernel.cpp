#include "CTRPluginFrameworkImpl/System/Kernel.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "CTRPluginFrameworkImpl/System/SystemImpl.hpp"
#include "CTRPluginFramework/System/Lock.hpp"
#include "ctrulib/svc.h"
#include "csvc.h"

#define THREADVARS_MAGIC  0x21545624 // !TV$

extern "C"
{
    void memcpy (void *dest, const void *source, size_t n);
}

using CTRPluginFramework::Lock;
using CTRPluginFramework::SystemImpl;
using CTRPluginFramework::ProcessImpl;

namespace Kernel
{
    static KRecursiveLock * CriticalSectionLock; // = (KRecursiveLock *)0xFFF2F0AC;
    static KThread  **  CurrentKThread = (KThread **)0xFFFF9000;
    static KProcess **  CurrentKProcess = (KProcess **)0xFFFF9004;
    static KScheduler** CurrentScheduler  = (KScheduler **)0xFFFF900C;

    static KCoreContext * CoreCtxs; // (KCoreContext *)0xFFFC9000;

    static void    (*KRecursiveLock__Lock)(KRecursiveLock *lock); // = (void (*)(KRecursiveLock *))0xFFF1DC24;
    static void    (*KRecursiveLock__Unlock)(KRecursiveLock *lock); // = (void (*)(KRecursiveLock *))0xFFF1DD64;
    static void    (*KScheduler__AdjustThread)(KScheduler *scheduler, KThread *thread, u32 oldSchedulingMask);// = (void (*)(KScheduler *, KThread*, u32))0xFFF1DD38;

    static inline u32   decodeARMBranch(const u32 *src)
    {
        s32 off = (*src & 0xFFFFFF) << 2;
        off = (off << 6) >> 6; // sign extend

        return (u32)src + 8 + off;
    }

    static void    K__Initialize(void)
    {
        __asm__ volatile("cpsid aif");

        u32 *   offset = (u32 *)0xFFF10000;

        // Search for KScheduler__AdjustThread
        for (; offset < (u32 *)0xFFF30000; ++offset)
        {
            if (offset[0] == 0xE5D13034 && offset[1] == 0xE1530002)
            {
                KScheduler__AdjustThread = (void (*)(KScheduler *, KThread*, u32))offset;

                // Get KCoreContext location
                offset = (u32 *)decodeARMBranch(offset + 7) + 3;
                CoreCtxs = (KCoreContext *)(offset[2 + ((*offset & 0xFF) >> 2)] - 0x8000);
                break;
            }
        }

        //u32  pattern[3] = { 0xE92D4010, 0xE1A04000, 0xE59F0034 };
        offset = (u32 *)0xFFF00000;
        for (; offset < (u32 *)0xFFF30000; ++offset)
        {
            if (*offset == 0xE92D4010 && *(offset + 1) == 0xE1A04000 && *(offset + 2) == 0xE59F0034)
            {
                offset += 2;
                CriticalSectionLock = (KRecursiveLock *)(offset[2 + (*offset & 0xFF) >> 2]);
                ++offset;
                KRecursiveLock__Lock = (void (*)(KRecursiveLock *))decodeARMBranch(offset);
                offset += 11;
                KRecursiveLock__Unlock = (void (*)(KRecursiveLock *))decodeARMBranch(offset);
                break;
            }
        }
    }

    void    Initialize(void)
    {
        svcCustomBackdoor((void *)K__Initialize);
    }

    static void     KThread__Lock(KThread *thread, bool mustLock)
    {
        KCoreObjectContext  &ctx = CoreCtxs[thread->coreId].objectContext;

        if (thread == *CurrentKThread || thread == ctx.currentThread)
            return;

        Lock    lock(CriticalSectionLock);

        u32     oldSchedulingMask = thread->schedulingMask;

        if (mustLock)
            thread->schedulingMask |= 0x40;
        else
            thread->schedulingMask &= ~0x40;

        ctx.currentScheduler->AdjustThread(thread, oldSchedulingMask);
    }

    void    Memcpy(void *dst, const void *src, const u32 size)
    {
        svcCustomBackdoor((void *)memcpy, dst, src, size);
    }

    u32     GetCurrentCoreId(void)
    {
        u32    (*K_GetCurrentCoreId)(void) = [](void) -> u32
        {
            u32 coreId;
            __asm__ __volatile__("mrc p15, 0, %0, c0, c0, 5" : "=r"(coreId));
            return coreId & 3;
        };

        return svcCustomBackdoor((void *)K_GetCurrentCoreId);
    }
}

bool    KAutoObject::IsKThread(void)
{
    u32     (*K_IsKThread)(KAutoObject *) = [](KAutoObject *obj) -> u32
    {
        KClassToken token;

        obj->vtable->GetClassToken(&token, obj);
        return token.flags == 0x8D;
    };

    return (bool)svcCustomBackdoor((void *)K_IsKThread, this);
}

void    KThread::Lock(void)
{
    svcCustomBackdoor((void *)Kernel::KThread__Lock, this, true);
}

void    KThread::Unlock(void)
{
    svcCustomBackdoor((void *)Kernel::KThread__Lock, this, false);
}

u32 *   KThread::GetTls(void)
{
    u32     (*KFunc)(KThread *) = [](KThread *thread) -> u32
    {
        return thread->tls;
    };

    return (u32 *)svcCustomBackdoor((void *)KFunc, this);
}

bool    KThread::IsPluginThread(void)
{
    return *GetTls() == THREADVARS_MAGIC;
}

KProcess * KProcess::GetCurrent(void)
{
    KProcess *(*K_GetCurrent)(void) = [](void) -> KProcess *
    {
        return *Kernel::CurrentKProcess;
    };

    return (KProcess *)svcCustomBackdoor((void *)K_GetCurrent);
}

void    KProcess::PatchCore2Access(void)
{
    if (!SystemImpl::IsNew3DS)
        return;

    void    (*K_PatchCore2Access)(KProcess *) = [](KProcess *process)
    {
        u32 *   kernelFlags = (u32 *)((u32)process + 0xB0);

        *kernelFlags |= 0x2000;
    };

    svcCustomBackdoor((void *)K_PatchCore2Access, this);
}

// C function for thread.c
extern "C" u32  KProcess__PatchCategory(u32 newCatagory)
{
    return ProcessImpl::KProcessPtr->PatchCategory(newCatagory);
}

u32     KProcess::PatchCategory(u32 newCategory)
{
    u32     (*K_PatchCategory)(KProcess *, u32, u32) = [](KProcess *process, u32 newCategory, u32 offset) -> u32
    {
        u32 *   category = (u32 *)((u32)process + offset);
        u32     old = *category & 0xF00;

        *category &= ~0xF00;
        *category |= newCategory;
        return old;
    };

    return svcCustomBackdoor((void *)K_PatchCategory, this, newCategory, SystemImpl::IsNew3DS ? 0xB0 : 0xA8);
}

// C function for thread.c
extern "C" u32     KProcess__PatchMaxPriority(u32 newPrio)
{
    return ProcessImpl::KProcessPtr->PatchMaxPriority(newPrio);
}

u32     KProcess::PatchMaxPriority(u32 newPrio)
{
    u32     (*K_PatchMaxPriority)(KProcess *, u32, u32) = [](KProcess *process, u32 newPrio, u32 offset) -> u32
    {
        KResourceLimit  *resLimit = (KResourceLimit *)*(u32 *)((u32)process + offset);
        u32  oldPrio = resLimit->maxPriority;

        resLimit->maxPriority = newPrio;
        return oldPrio;
    };

    return svcCustomBackdoor((void *)K_PatchMaxPriority, this, newPrio, SystemImpl::IsNew3DS ? 0x84 : 0x7C);
}

void    KScheduler::AdjustThread(KThread *thread, u32 oldSchedulingMask)
{
    Kernel::KScheduler__AdjustThread(this, thread, oldSchedulingMask);
}

void    KRecursiveLock::Lock(void)
{
    Kernel::KRecursiveLock__Lock(this);
}

void    KRecursiveLock::Unlock(void)
{
    Kernel::KRecursiveLock__Unlock(this);
}

#include "CTRPluginFrameworkImpl/System/Kernel.hpp"
#include "ctrulib/svc.h"
#include "csvc.h"

#define THREADVARS_MAGIC  0x21545624 // !TV$

extern "C"
{
    void memcpy (void *dest, const void *source, size_t n);
}

namespace Kernel
{
    static u32          CriticalSectionLock = 0xFFF2F0AC;
    static KThread  *   CurrentKThread = (KThread *)0xFFFF9000;
    static KProcess *   CurrentKProcess = (KProcess *)0xFFFF9004;
    static KScheduler * CurrentScheduler  = (KScheduler *)0xFFFF900C;

    static KCoreContext * CoreCtxs = (KCoreContext *)0xFFFC9000;

    static void    (*KRecursiveLock__Lock)(u32 lock) = (void (*)(u32))0xFFF1DC24;
    static void    (*KRecursiveLock__Unlock)(u32 lock) = (void (*)(u32))0xFFF1DD64;
    static void    (*KScheduler__AdjustThread)(KScheduler *scheduler, KThread *thread, u32 oldSchedulingMask) = (void (*)(KScheduler *, KThread*, u32))0xFFF1DD38;

    static void     KThread__Lock(KThread *thread, bool lock)
    {
        KCoreObjectContext &ctx = CoreCtxs[thread->coreId].objectContext;

        if (thread == CurrentKThread || thread == ctx.currentThread)
            return;

        KRecursiveLock__Lock(CriticalSectionLock);

        u32     oldSchedulingMask = thread->schedulingMask;

        if (lock)
            thread->schedulingMask |= 0x40;
        else
            thread->schedulingMask &= ~0x40;

        ctx.currentScheduler->AdjustThread(thread, oldSchedulingMask);

        KRecursiveLock__Unlock(CriticalSectionLock);
    };

    void    Memcpy(void *dst, const void *src, const u32 size)
    {
        svcCustomBackdoor((void *)memcpy, dst, src, size);
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

void    KScheduler::AdjustThread(KThread *thread, u32 oldSchedulingMask)
{
    Kernel::KScheduler__AdjustThread(this, thread, oldSchedulingMask);
}

#include "CTRPluginFrameworkImpl/arm11kCommands.h"
#include "3DS.h"
#include "csvc.h"

u32     g_kernelParams[32];
u32     g_KProcessHandleDataOffset;
u32     g_KProcessKernelFlagsOffset;
u32     g_KProcessPIDOffset = 0xBC;

typedef struct
{
  u32   info;
  u32   kobj;
}       HandleDescriptor;

typedef struct
{
  HandleDescriptor  *handleTable;
  s16               maxHandleCount;
  s16               highestHandleCount;
  HandleDescriptor  *nextOpenHandleDescriptor;
  s16               totalHandlesUsed;
  s16               handlesInUseCount;
}       KProcessHandleTable;

typedef s32 (*KCallback)(void);

u32     K_GetKObjFromHandle(KProcessHandleTable *kProcHandleTable, Handle handle)
{
    return kProcHandleTable->handleTable[handle & 0xffff].kobj;
}

u32     K_GetCurrentKProcess(void)
{
    return *(u32 *)0xFFFF9004;
}

u32     K_GetCurrentKThread(void)
{
    return *(u32 *)0xFFFF9000;
}

u32 arm11kSetKProcessId(u32 kprocess, u32 newPid)
{
    g_kernelParams[0] = 5;
    g_kernelParams[1] = kprocess;
    g_kernelParams[2] = newPid;
    g_kernelParams[3] = 1;
    svcBackdoor(executeKernelCmd);
    return (g_kernelParams[1]);
}

u32 arm11kGetKProcessId(u32 kprocess)
{
    g_kernelParams[0] = 5;
    g_kernelParams[1] = kprocess;
    g_kernelParams[3] = 0;
    svcBackdoor(executeKernelCmd);
    return (g_kernelParams[1]);
}

u32 arm11kSetCurrentKProcess(u32 kprocess)
{
    g_kernelParams[0] = 4;
    g_kernelParams[1] = kprocess;
    svcBackdoor(executeKernelCmd);
    return (g_kernelParams[2]);
}

u32 arm11kGetCurrentKProcess(void)
{
    g_kernelParams[0] = 3;
    svcBackdoor(executeKernelCmd);
    return (g_kernelParams[1]);
}

static u32  K_Memcpy(u32 dst_, u32 src_, u32 size)
{
    __asm__ __volatile__("cpsid aif");

    u8 *dst = (u8 *)dst_;
    u8 *src = (u8 *)src_;

    while (size--)
        *dst++ = *src++;

    return 0;
}

void    arm11kMemcpy(void *dst, const void *src, const u32 size)
{
    svcCustomBackdoor(K_Memcpy, dst, src, size);
}

static u32     K_PatchAllowCore2(void)
{
    __asm__ __volatile__("cpsid aif");

    u32     kprocess = K_GetCurrentKProcess();
    u32 *   kernelFlags = (u32 *)(kprocess + 0xB0);

    *kernelFlags |= 0x2000;

    return 0;
}

void     arm11kAllowCore2(void)
{
    svcCustomBackdoor(K_PatchAllowCore2);
}

static u32  K_EditKProcessAppCategory(u32 newType, u32 kernelFlagsOffset)
{
    __asm__ __volatile__("cpsid aif");

    u32     kproc = K_GetCurrentKProcess();
    u32 *   category = (u32 *)(kproc + kernelFlagsOffset);
    u32     old = *category & 0xF00;

    *category &= ~0xF00;
    *category |= newType;
    return old;
}

u32     arm11kChangeProcessType(u32 newType)
{
    return svcCustomBackdoor(K_EditKProcessAppCategory, newType, g_KProcessKernelFlagsOffset);
}

static u32    K_getCurrentCoreId(void)
{
    u32 coreId;
    __asm__ __volatile__("cpsid aif");
    __asm__ __volatile__("mrc p15, 0, %0, c0, c0, 5" : "=r"(coreId));
    return coreId & 3;
}

u32     arm11kGetCurrentCoreID(void)
{
    return svcCustomBackdoor(K_getCurrentCoreId);
}

Result  arm11kSvcControlMemory(u32 *addr, u32 addr1, u32 size, u32 op, u32 perm)
{
    u32     currentKProcess = arm11kGetCurrentKProcess();
    u32     currentPID = arm11kSetKProcessId(currentKProcess, 1);
    u32     temp;
    Result res = svcControlMemory(addr, addr1, 0x0, size, op, perm);
    arm11kSetKProcessId(currentKProcess, currentPID);

    return (res);
}

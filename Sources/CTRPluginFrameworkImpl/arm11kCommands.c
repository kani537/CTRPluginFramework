#include "CTRPluginFrameworkImpl/arm11kCommands.h"
#include "3DS.h"

u32     g_kernelParams[32];
u32     g_KProcessHandleDataOffset;
u32     g_KProcessPIDOffset = 0xBC;

u32 arm11kSetKProcessId(u32 kprocess, u32 newPid)
{
    g_kernelParams[0] = 5;
    g_kernelParams[1] = kprocess;
    g_kernelParams[2] = newPid;
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

u32 arm11kGetKProcessFromHandle(Handle processHandle)
{
    g_kernelParams[0] = 2;
    g_kernelParams[1] = processHandle;
    svcBackdoor(executeKernelCmd);
    return (g_kernelParams[2]);
}

u32 arm11kGetKProcessState(u32 kProcessState)
{
    g_kernelParams[0] = 6;
    g_kernelParams[1] = kProcessState;
    svcBackdoor(executeKernelCmd);
    return (g_kernelParams[2] & 0xFF);
}

void arm11kMemcpy(u32 dst, u32 src, u32 size)
{
  g_kernelParams[0] = 1;
  g_kernelParams[1] = dst;
  g_kernelParams[2] = src;
  g_kernelParams[3] = size;
  svcBackdoor(executeKernelCmd);
  return;
}

Result  arm11kSvcControlMemory(u32 *addr, u32 addr1, u32 size, u32 op, u32 perm)
{
  u32 currentKProcess;
  u32 currentPID;
  Result res;

  currentKProcess = arm11kGetCurrentKProcess();
  currentPID = arm11kSetKProcessId(currentKProcess, 1);
  u32 temp;
  res = svcControlMemory(addr, addr1, 0x0, size, op, perm);
  arm11kSetKProcessId(currentKProcess, currentPID);
  return (res);
}

u32 getKernelObjectPtr(void *KProcessHandleTable, Handle processHandle)
{
  return *(u32 *)(*(u32 *)KProcessHandleTable + (8 * (processHandle & 0x3FFFF)) + 4);
}


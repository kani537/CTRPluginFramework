#ifndef ARM11K_COMMANDS_H
#define ARM11K_COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/**
asm
**/
s32     executeKernelCmd(void);

// Set a new pid to the KObject and return the old one
u32     arm11kSetKProcessId(u32 kprocess, u32 newPid);

// Change the current KProcess and return the old one
u32     arm11kSetCurrentKProcess(u32 kprocess);

// Return current KProcess
u32     arm11kGetCurrentKProcess(void);

// Return KProcess associated to the handle
u32     arm11kGetKProcessFromHandle(Handle processHandle);

// Return the status read at kProcessState
u32     arm11kGetKProcessState(u32 kProcessState);

// Do a memcpy in kernel mode
void arm11kMemcpy(u32 dst, u32 src, u32 size);

//
Result  arm11kSvcControlMemory(u32 *addr, u32 addr1, u32 size, u32 op, u32 perm);


u32 getKernelObjectPtr(void *KProcessHandleTable, Handle processHandle);

#ifdef __cplusplus
}
#endif
#endif
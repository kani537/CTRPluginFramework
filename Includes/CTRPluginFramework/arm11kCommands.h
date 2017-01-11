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

// Do a memcpy in kernel mode
void arm11kMemcpy(u32 dst, u32 src, u32 size);


u32 getKernelObjectPtr(void *KProcessHandleTable, Handle processHandle);

#ifdef __cplusplus
}
#endif
#endif
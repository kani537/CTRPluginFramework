#ifndef HOOK_H
#define HOOK_H

#include "types.h"

#define HookReturn(hook) __asm__ __volatile__("mov pc,%0" :: "r" (hook.returnCode))

struct  Hook
{
    bool        isInitialized;
    bool        isEnabled;
    u32         targetAddress;
    u32         afterHookAddress;
    u32         targetCode[2];
    u32         jumpCode[2];
    u32         returnCode[4];

    Hook();
    void        Initialize(u32 targetAddr, u32 callbackAddr);
    void        DeInitialize(void);
    void        Enable(void);
    void        Disable(void);
};

#endif

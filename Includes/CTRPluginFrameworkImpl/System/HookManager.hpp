#ifndef CTRPLUGINFRAMEWORKIMPL_SYSTEM_HOOKMANAGER_HPP
#define CTRPLUGINFRAMEWORKIMPL_SYSTEM_HOOKMANAGER_HPP

#include "types.h"
#include "CTRPluginFramework/System/Clock.hpp"
#include "CTRPluginFramework/System/Mutex.hpp"

namespace CTRPluginFramework
{
    #define MAX_HOOK_WRAPPERS 93

    struct HookWrapper
    {
        u32     backupAndUpdateLR;  ///< Default: nop
        u32     overwrittenInstr;   ///< Default: nop
        u32     setLR[2];           ///< Default: nop
        u32     jumpToCallback;     ///< ldr pc, [pc, #-4]
        u32     callbackAddress;    ///< Address of the code to jump to
        u32     restoreLR;          ///< Default: nop
        u32     overwrittenInstr2;  ///< Default: nop
        u32     jumpBackToGame;     ///< ldr pc, [pc, #-4]
        u32     returnAddress;
        u32     lrBackup;
    };

    struct HookWrapperStatus
    {
        bool            isEnabled{false};
        u32             target{0};
        Clock           disabledSince{ CTRPluginFramework::Microseconds(0)};
        HookWrapper     *wrapper{nullptr};
    };

    struct HookManager
    {
        static bool     Init(void);
        static Mutex&   Lock(void);

        // Return index free or -1 if error
        static int      AllocNewHook(u32 address = 0);

        // Free a hook
        static void     FreeHook(u32 &index);

        Mutex               lock;
        HookWrapperStatus   hws[MAX_HOOK_WRAPPERS];

        static HookManager *instance;
    };
}

#endif

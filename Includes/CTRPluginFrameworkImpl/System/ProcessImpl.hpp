#ifndef CTRPLUGINFRAMEWORKIMPL_PROCESSIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_PROCESSIMPL_HPP

#include "ctrulib/svc.h"
#include "ctrulib/synchronization.h"
#include "CTRPluginFrameworkImpl/System/Kernel.hpp"
#include <vector>

namespace CTRPluginFramework
{
    class ProcessImpl
    {
    public:
        // Pause the current process
        static void     Pause(bool useFading);
        // Unpause the current process
        static void     Play(bool forced);

        static void     Initialize(void);

        static bool     PatchProcess(u32 addr, u8 *patch, u32 length, u8 *original);
        static void     GetHandleTable(KProcessHandleTable &table, std::vector<HandleDescriptor> &handleDescriptors);
        static void     GetGameThreads(std::vector<KThread *> &threads);
        static void     LockGameThreads(void);
        static void     UnlockGameThreads(void);

        static Handle       ProcessHandle;
        static u32          IsPaused;
        static u32          ProcessId;
        static u64          TitleId;

        static KThread *    MainThread;
        static KProcess *   KProcessPtr;
        static KCodeSet     CodeSet;
    };
}

#endif

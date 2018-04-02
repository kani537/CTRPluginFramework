#if __INTELLISENSE__
typedef unsigned int __SIZE_TYPE__;
typedef unsigned long __PTRDIFF_TYPE__;
//#undef __cplusplus
//#define __cplusplus 201103L
#undef __cpp_exceptions
#define __cpp_exceptions 0
#define __attribute__(q)
#define __extension__
#define __asm__(expr)
#define __builtin_labs(a) 0
#define __builtin_llabs(a) 0
#define __builtin_fabs(a) 0
#define __builtin_fabsf(a) 0
#define __builtin_fabsl(a) 0
#define __builtin_strcmp(a,b) 0
#define __builtin_strlen(a) 0
#define __builtin_memcpy(a,b) 0
#define __builtin_va_list void*
#define __builtin_va_start(a,b)
#endif

#include "Hook.hpp"
#include "3DS.h"
#include "CTRPluginFramework.hpp"
#include <string>

namespace CTRPluginFramework
{
    // This function is called on the plugin starts, before main
    void    PatchProcess(FwkSettings &settings)
    {
        settings.WaitTimeToBoot = Seconds(10.f);
        settings.EcoMemoryMode = true;
        if (System::IsLoaderNTR())
            settings.HeapSize = 0x150000;
    }

#define DEBUG 1
#if DEBUG
#   define TRACE Debug_Trace()
#else
#   define TRACE
#endif
    static std::string g_abortDebugStr;

    void    Debug_Trace(void)
    {
        g_abortDebugStr = Utils::Format("%s: %s:%d", __FILE__, __FUNCTION__, __LINE__);
    }

    void    OnAbort(void)
    {
        OSD::Run([](const Screen &screen)
        {
            if (screen.IsTop)
                return false;

            screen.Draw("std::abort!!!", 10, 10, Color::Blank, Color::Red);
            screen.Draw("Last function: " + g_abortDebugStr, 10, 20, Color::Blank, Color::Black);
            return true;
        });

        for (;;); // Don't exit this function or the plugin will be shutdown (threads stopped and OSD unhooked)
    }

    int     main(void)
    {
        PluginMenu  *m = new PluginMenu("Action Replay", 1, 0, 5);
        PluginMenu  &menu = *m;

        OSD::Run([](const Screen &screen)
        {
            if (!screen.IsTop) return false;

            screen.Draw(Utils::Format("Free: %08X", getMemFree()), 10, 10);

            return true;
        });
        menu.SyncronizeWithFrame(true);

#if DEBUG
        System::OnAbort = OnAbort;
#endif

        // Launch menu and mainloop
        int ret = menu.Run();

        delete m;
        // Exit plugin
        return (0);
    }
}

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
    }

    void    CBAbort(void)
    {

    }

    int     main(void)
    {
        PluginMenu  *m = new PluginMenu("Action Replay", 1, 0, 5);
        PluginMenu  &menu = *m;

        menu.SyncronizeWithFrame(true);

        System::OnAbort = CBAbort;

        // Launch menu and mainloop
        int ret = menu.Run();

        delete m;
        // Exit plugin
        return (0);
    }
}

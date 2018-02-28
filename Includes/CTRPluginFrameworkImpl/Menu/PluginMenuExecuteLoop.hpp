#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUEXECUTELOOP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUEXECUTELOOP

#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"

#include <vector>
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "3DS.h"

namespace CTRPluginFramework
{
    class MenuEntryImpl;
    class MenuEntryActionReplay;
    class PluginMenuExecuteLoop
    {
    public:
        PluginMenuExecuteLoop(void);
        ~PluginMenuExecuteLoop(void) = default;

        static void WriteEnabledCheatsToFile(Preferences::Header &header, File &file);

        static void     Add(MenuEntryImpl *entry);
        static void     Remove(MenuEntryImpl *entry);
        static void     ExecuteBuiltin(void);
        static void     Lock(void);
        static void     Unlock(void);

        static void     AddAR(MenuEntryActionReplay *entry);
        static void     RemoveAR(MenuEntryActionReplay *entry);
        static void     ExecuteAR(void);
        static void     LockAR(void);
        static void     UnlockAR(void);

        bool    operator()(void);

        static void InitLocks(void);

    private:
        static PluginMenuExecuteLoop          * _firstInstance;
        static LightLock                        _arLock;
        static LightLock                        _builtinLock;

        std::vector<MenuEntryImpl *>            _builtinEnabledList{};
        std::vector<MenuEntryActionReplay *>    _arEnabledList{};
    };
}

#endif

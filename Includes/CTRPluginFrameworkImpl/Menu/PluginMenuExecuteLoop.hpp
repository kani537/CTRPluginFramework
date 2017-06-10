#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUEXECUTELOOP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUEXECUTELOOP

#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"

#include <vector>
#include <queue>
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    class MenuEntryImpl;
    class PluginMenuExecuteLoop
    {   
    public:
        PluginMenuExecuteLoop(void);
        ~PluginMenuExecuteLoop(void) {}
        static void WriteEnabledCheatsToFile(Preferences::Header &header, File &file);

        static void    Add(MenuEntryImpl *entry);
        static void    Remove(MenuEntryImpl *entry);

        bool    operator()(void);

    private:
        static PluginMenuExecuteLoop    *_firstInstance;
        std::vector<MenuEntryImpl *>    _executeLoop;
        std::queue<int>                 _availableIndex;

    };
}

#endif
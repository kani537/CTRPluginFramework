#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUEXECUTELOOP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUEXECUTELOOP

#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"

#include <vector>

namespace CTRPluginFramework
{
    class MenuEntryImpl;
    class PluginMenuExecuteLoop
    {   
    public:
        PluginMenuExecuteLoop(void);
        ~PluginMenuExecuteLoop(void) {};

        static void    Add(MenuEntryImpl *entry);
        static void    Remove(MenuEntryImpl *entry);

        bool    operator()(void);

    private:
        static PluginMenuExecuteLoop    *_firstInstance;
        std::vector<MenuEntryImpl *>    _executeLoop;
    };
}

#endif
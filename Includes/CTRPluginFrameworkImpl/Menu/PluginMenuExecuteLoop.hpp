#ifndef CTRPLUGINFRAMEWORK_PLUGINMENUEXECUTELOOP
#define CTRPLUGINFRAMEWORK_PLUGINMENUEXECUTELOOP

#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"

#include <vector>

namespace CTRPluginFramwork
{
    class PluginMenuExecuteLoop
    {
    public:
        PluginMenuExecuteLoop(void);
        ~PluginMenuExecuteLoop(void) {};

        void    Add(MenuEntryImpl *entry);
        void    Remove(MenuEntryImpl *entry);

        bool    operator()(void);

    private:

        std::vector<MenuEntryImpl *>    _executeLoop;
    };
}

#endif
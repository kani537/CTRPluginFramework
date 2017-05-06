#ifndef CTRPLUGINFRAMEWORK_PLUGINMENU_HPP
#define CTRPLUGINFRAMEWORK_PLUGINMENU_HPP

#include "CTRPluginFramework/Menu/MenuEntry.hpp"
#include "CTRPluginFramework/Menu/MenuFolder.hpp"

#include <string>
#include <memory>

namespace CTRPluginFramework
{
    class PluginMenuImpl;
    class PluginMenu
    {
        using CallbackPointer = void (*)(void);
    public:

        PluginMenu(std::string name = "Cheats", std::string note = "");
        ~PluginMenu(void){};

        void    Append(MenuEntry *item) const;
        void    Append(MenuFolder *item) const;
        void    Callback(CallbackPointer callback) const;
        int     Run(void) const;
        void    SetSearchButtonState(bool isEnabled) const;
        void    SetActionReplayButtonState(bool isEnabled) const;

    private:
        
        PluginMenuImpl *_menu;
        //std::unique_ptr<PluginMenuImpl> _menu;
    };
}

#endif
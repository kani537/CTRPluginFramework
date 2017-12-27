#ifndef CTRPLUGINFRAMEWORK_PLUGINMENUACTIONREPLAY_HPP
#define CTRPLUGINFRAMEWORK_PLUGINMENUACTIONREPLAY_HPP

#include "types.h"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"

namespace CTRPluginFramework
{
    class PluginMenuActionReplay
    {
        using EventList = std::vector<Event>;
    public:
        PluginMenuActionReplay();
        ~PluginMenuActionReplay();

        // Initialize and load cheats
        void    Initialize(void);

        // Display menu
        // Return true if menu must close
        bool    operator()(EventList &eventList, const Time &delta);

    private:
        Menu    _topMenu;
        ToggleButton<PluginMenuActionReplay, void>      _noteBtn;
        IconButton<PluginMenuActionReplay, void>        _editorBtn;

        void    _DrawBottom(void);
        void    _ProcessEvent(EventList &eventList);
        void    _Update(const Time &delta);
        void    _EditorBtn_OnClick(void);
    };
}

#endif
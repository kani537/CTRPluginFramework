#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUTOOLS_HPP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUTOOLS_HPP

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuFolderImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuItem.hpp"

#include <vector>
#include <string>

namespace CTRPluginFramework
{
    class PluginMenuTools
    {
        using EventList = std::vector<Event>;
    public:
        PluginMenuTools(std::string &about);
        ~PluginMenuTools(){}

        // Return true if the Close Button is pressed, else false
        bool    operator()(EventList &eventList, Time &delta);
    private:

        void    _ProcessEvent(Event &event);
        void    _RenderTop(void);
        void    _RenderTopMenu(void);
        void    _RenderBottom(void);
        void    _Update(Time delta);



        // Members
        //Menu                            _menu;
        std::string     _about;

        // Buttons        
        IconButton<PluginMenuTools, void>          _closeBtn;

    };
}

#endif
#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUSEARCH_HPP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUSEARCH_HPP

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuFolderImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuItem.hpp"

#include <vector>

namespace CTRPluginFramework
{
    class PluginMenuSearch
    {
        using EventList = std::vector<Event>;
    public:
        PluginMenuSearch();
        ~PluginMenuSearch(){}

        // Return true if the Close Button is pressed, else false
        bool    operator()(EventList &eventList, Time &delta);
    private:

        void    _ProcessEvent(Event &event);
        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _Update(Time delta);



        // Members
        //Menu                            _menu;


        // Buttons        
        IconButton<PluginMenuSearch, void>          _closeBtn;

    };
}

#endif
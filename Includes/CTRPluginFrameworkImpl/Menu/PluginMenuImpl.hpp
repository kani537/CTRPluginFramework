#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUIMPL_HPP

#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/GuideReader.hpp"

#include "CTRPluginFrameworkImpl/Menu/PluginMenuHome.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"

#include <queue>

namespace CTRPluginFramework
{
    class PluginMenuImpl
    {

    public:

        PluginMenuImpl(std::string name = "Cheats", std::string note = "");
        ~PluginMenuImpl(void);

        void    Append(MenuItem *item);
        int     Run(void);
        void    Null(void);
    private: 


        bool                        _isOpen;
        bool                        _pluginRun;
        
        PluginMenuHome              *_home;
        PluginMenuExecuteLoop       *_executeLoop;
        GuideReader                *_guide;
        void                        _TriggerGuide(void);
    };
}

#endif
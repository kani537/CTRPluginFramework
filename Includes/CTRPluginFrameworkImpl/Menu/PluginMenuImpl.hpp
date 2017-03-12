#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUIMPL_HPP

#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/GuideReader.hpp"

#include "CTRPluginFrameworkImpl/Menu/PluginMenuHome.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearch.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuTools.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"

#include <vector>

namespace CTRPluginFramework
{
    class PluginMenuImpl
    {
    public:

        PluginMenuImpl(std::string name = "Cheats", std::string note = "");
        ~PluginMenuImpl(void);

        void    Append(MenuItem *item);
        void    Callback(CallbackPointer callback);
        int     Run(void);
        void    Null(void);
    private: 


        bool                        _isOpen;
        bool                        _pluginRun;
        
        PluginMenuHome              *_home;
        PluginMenuSearch            *_search;
        PluginMenuTools             *_tools;
        PluginMenuExecuteLoop       *_executeLoop;
        GuideReader                 *_guide;
        void                        _TriggerGuide(void);
        std::vector<CallbackPointer>     _callbacks;
    };
}

#endif
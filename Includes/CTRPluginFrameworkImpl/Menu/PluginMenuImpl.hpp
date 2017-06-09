#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUIMPL_HPP

#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/GuideReader.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuFreeCheats.hpp"

#include "CTRPluginFrameworkImpl/Menu/PluginMenuHome.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearch.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuTools.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

#include <vector>

namespace CTRPluginFramework
{
    class PluginMenuImpl
    {
    public:

        PluginMenuImpl(std::string name = "Cheats", std::string note = "");
        ~PluginMenuImpl(void);

        void    Append(MenuItem *item) const;
        void    Callback(CallbackPointer callback);
        int     Run(void);

        static void LoadEnabledCheatsFromFile(const Preferences::Header &header, File &settings);
        static void LoadFavoritesFromFile(const Preferences::Header &header, File &settings);

        static void WriteEnabledCheatsToFile(Preferences::Header &header, File &settings);
        static void WriteFavoritesToFile(Preferences::Header &header, File &settings);

        // Used to forcefully exit a menu
        static void ForceExit(void);

        static void UnStar(MenuItem *item);
        static void Refresh(void);

        void    TriggerSearch(bool state) const;
        void    TriggerActionReplay(bool state) const;
        void    TriggerFreeCheats(bool isEnabled) const;

        bool    IsOpen(void) const;
        bool    WasOpened(void) const;
    private: 

        static PluginMenuImpl       *_runningInstance;

        bool                        _isOpen;
        bool                        _wasOpened;
        bool                        _pluginRun;
        
        PluginMenuHome              *_home;
        PluginMenuSearch            *_search;
        PluginMenuTools             *_tools;
        PluginMenuExecuteLoop       *_executeLoop;
        GuideReader                 *_guide;
        HexEditor                   _hexEditor;
        FreeCheats                  _freeCheats;
        std::vector<CallbackPointer>     _callbacks;
    };
}

#endif
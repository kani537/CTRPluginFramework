#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUIMPL_HPP

#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/GuideReader.hpp"

#include "CTRPluginFrameworkImpl/Menu/PluginMenuHome.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearch.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuTools.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

#include <vector>
#include "PluginMenuActionReplay.hpp"

namespace CTRPluginFramework
{
    class PluginMenuImpl
    {
        using FuncPointer = void(*)(void);
        using HotkeysVector = std::vector<Preferences::HotkeysInfos>;
    public:

        PluginMenuImpl(std::string &name, std::string &note);
        ~PluginMenuImpl(void);

        void    Append(MenuItem *item) const;
        void    Callback(CallbackPointer callback);
        void    RemoveCallback(CallbackPointer callback);
        int     Run(void);

        static void    Close(MenuFolderImpl *menuFolderImpl);

        static void LoadEnabledCheatsFromFile(const Preferences::Header &header, File &settings);
        static void LoadFavoritesFromFile(const Preferences::Header &header, File &settings);
        static void LoadHotkeysFromFile(const Preferences::Header &header, File &settings);

        static void WriteEnabledCheatsToFile(Preferences::Header &header, File &settings);
        static void WriteFavoritesToFile(Preferences::Header &header, File &settings);
        static void ExtractHotkeys(HotkeysVector &hotkeys, MenuFolderImpl *folder, u32 &size);
        static void WriteHotkeysToFile(Preferences::Header &header, File &file);
        static void GetRegionsList(std::vector<Region> &list);
        // Used to forcefully exit a menu
        static void ForceExit(void);
        static void ForceOpen(void);

        static void UnStar(MenuItem *item);
        static void Refresh(void);

        void    SetHexEditorState(bool isEnabled) const;
        void    ShowWelcomeMessage(bool showMsg);

        MenuFolderImpl *GetRoot(void) const;
        bool    IsOpen(void) const;
        bool    WasOpened(void) const;
        void    AddPluginVersion(u32 version) const;

        FuncPointer     OnFirstOpening;
        FuncPointer     OnOpening;
        bool            SyncOnFrame;
    private:

        static PluginMenuImpl       *_runningInstance;

        bool                        _isOpen;
        bool                        _wasOpened;
        bool                        _pluginRun;
        bool                        _showMsg;

        PluginMenuActionReplay      *_actionReplay;
        PluginMenuHome              *_home;
        PluginMenuSearch            *_search;
        PluginMenuTools             *_tools;
        PluginMenuExecuteLoop       *_executeLoop;
        GuideReader                 *_guide;
        HexEditor                   _hexEditor;
        std::vector<CallbackPointer>     _callbacks;
        bool                        _forceOpen;
    };
}

#endif

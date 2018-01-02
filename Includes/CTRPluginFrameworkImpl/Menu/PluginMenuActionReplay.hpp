#ifndef CTRPLUGINFRAMEWORK_PLUGINMENUACTIONREPLAY_HPP
#define CTRPLUGINFRAMEWORK_PLUGINMENUACTIONREPLAY_HPP

#include "types.h"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARCodeEditor.hpp"

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

        static void     SaveCodes(void);
        static void     NewARCode(u8 type, u32 address, u32 value);
    private:
        ARCodeEditor    _editor;
        Menu            _topMenu;
        using ToggleBtn = ToggleButton<PluginMenuActionReplay, void>;
        using IconBtn = IconButton<PluginMenuActionReplay, void>;

        ToggleBtn       _noteBtn;
        IconBtn         _editorBtn;
        IconBtn         _newBtn;
        IconBtn         _cutBtn;
        IconBtn         _pasteBtn;
        IconBtn         _duplicateBtn;
        IconBtn         _trashBtn;

        MenuItem        *_clipboard;

        void    _DrawBottom(void);
        void    _ProcessEvent(EventList &eventList);
        void    _Update(const Time &delta);
        void    _EditorBtn_OnClick(void);
        void    _NewBtn_OnClick(void);
        void    _CutBtn_OnClick(void);
        void    _PasteBtn_OnClick(void);
        void    _DuplicateBtn_OnClick(void);
        void    _TrashBtn_OnClick(void);
    };
}

#endif

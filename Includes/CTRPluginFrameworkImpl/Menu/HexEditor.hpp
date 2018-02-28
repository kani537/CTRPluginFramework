#ifndef CTRPLUGINFRAMEWORKIMPL_HEXEDITOR_HPP
#define CTRPLUGINFRAMEWORKIMPL_HEXEDITOR_HPP

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFrameworkImpl/Menu/KeyboardImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/SubMenu.hpp"

#include <vector>
#include <string>

namespace CTRPluginFramework
{
    class HexEditor
    {
        using EventList = std::vector<Event>;
    public:
        HexEditor(u32 target);
        ~HexEditor(){};

        // Return true if the user decided to close it
        bool    operator()(EventList &eventList);
        void    Goto(u32 address, bool updateCursor = false);

    private:

        void    _ProcessEvent(Event &event);
        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _Update(void);

        void    _CreateCheat(void);
        u32     _GetCursorAddress(void) const;

        void    _MoveBackward(void);
        void    _MoveForward(void);
        void    _SaveThisAddress(void);
        void    _BrowseHistory(void);
        void    _ClearHistory(void);
        void    _ApplyChanges(void);
        void    _DiscardChanges(void);
        void    _JumpTo(void);
        void    _GotoPreviousRegion(void);
        void    _GotoNextRegion(void);

        void    _GetChar(u8 *buffer, int offset);

        SubMenu                             _submenu;
        bool                                _invalid;
        bool                                _isModified;
        u8                                  *_memoryAddress;
        u32                                 _startRegion;
        u32                                 _endRegion;
        int                                 _cursor;
        int                                 _indexHistory;

        // Buffer for memory
        u8                                  _memory[256];

        // Keyboard
        KeyboardImpl                        _keyboard;

        // Buttons
        IconButton<HexEditor, void>         _closeBtn;

        std::vector<u32>                    _history;
    };
}

#endif
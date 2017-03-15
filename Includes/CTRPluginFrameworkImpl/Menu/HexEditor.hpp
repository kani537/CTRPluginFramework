#ifndef CTRPLUGINFRAMEWORKIMPL_HEXEDITOR_HPP
#define CTRPLUGINFRAMEWORKIMPL_HEXEDITOR_HPP

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFrameworkImpl/Menu/KeyboardImpl.hpp"

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
        void    Goto(u32 address);

    private:

        void    _ProcessEvent(Event &event);
        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _Update(void);

        
        void    _ApplyChanges(void);
        void    _DiscardChanges(void);

        std::string     _GetChar(int offset);

        bool                                _invalid;
        bool                                _isModified;
        u8                                  *_memoryAddress;
        int                                 _cursor;
        // Buffer for memory
        u8                                  _memory[256];

        // Keyboard
        KeyboardImpl                        _keyboard;

        // Buttons
        IconButton<HexEditor, void>         _closeBtn;


    };
}

#endif
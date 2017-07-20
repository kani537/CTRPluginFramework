#ifndef CTRPLUGINFRAMEWORK_TEXTBOX_HPP
#define CTRPLUGINFRAMEWORK_TEXTBOX_HPP

#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"

#include <string>
#include <vector>

namespace CTRPluginFramework
{
    class TextBox
    {
    public:
        TextBox(std::string title, std::string &text, IntRect box);
        ~TextBox(){}

        // Open the texbox
        void    Open(void);
        // Close the textbox
        void    Close(void);

        
        bool    IsOpen(void);

        // Process Event
        // return false if the tb is closed
        bool    ProcessEvent(Event &event);
        // Update
        void    Update(const std::string &title, std::string &text);
        // Draw
        void    Draw(void);

        Color   titleColor;
        Color   textColor;
        Color   borderColor;

    private:
        void    _GetTextInfos(void);
        u8      *_GetWordWidth(u8 *str, float& width);

        std::vector<u8 *>      _newline;
        std::string             _title;
        std::string             *_text;
        IntRect                 _box;
        IntRect                 _border;
        bool                    _isOpen;
        bool                    _displayScrollbar;

        u32                     _currentLine;
        u32                     _maxLines;
        u32                     _scrollbarSize;
        u32                     _scrollCursorSize;
        float                   _scrollPadding;
        float                   _scrollPosition;
        Clock                   _inputClock;
    };
}

#endif
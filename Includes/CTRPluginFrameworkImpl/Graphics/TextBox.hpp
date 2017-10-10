#ifndef CTRPLUGINFRAMEWORK_TEXTBOX_HPP
#define CTRPLUGINFRAMEWORK_TEXTBOX_HPP

#include "CTRPluginFrameworkImpl/Graphics/Drawable.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"

#include <string>
#include <vector>

namespace CTRPluginFramework
{
    class TextBox : public Drawable
    {
    public:
        TextBox(const std::string &title, const std::string &text, const IntRect &box);
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
        void    Draw(void) override;
        void    Update(const bool isTouchDown, const IntVector &pos) override;

        Color   titleColor;
        Color   textColor;
        Color   borderColor;

    private:
        void    _GetTextInfos(void);
        u8      *_GetWordWidth(u8 *str, float& width);

        std::vector<u8 *>       _newline;
        std::string             _title;
        const std::string       *_text;
        const IntRect           _box;
        const IntRect           _border;
        bool                    _isOpen;
        bool                    _displayScrollbar;

        u32                     _currentLine;
        u32                     _maxLines;
        u32                     _scrollbarSize;
        u32                     _scrollCursorSize;
        u32                     _maxScrollCursorPosY;
        float                   _scrollPadding;
        float                   _scrollPosition;
        Clock                   _inputClock;
    };
}

#endif
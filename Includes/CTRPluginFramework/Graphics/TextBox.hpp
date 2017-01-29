#ifndef CTRPLUGINFRAMEWORK_TEXTBOX_HPP
#define CTRPLUGINFRAMEWORK_TEXTBOX_HPP


#include "CTRPluginFramework/Vector.hpp"
#include "CTRPluginFramework/Rect.hpp"
#include "CTRPluginFramework/Screen.hpp"
#include "CTRPluginFramework/Events.hpp"
#include "CTRPluginFramework/Graphics/Renderer.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"

#include <string>

namespace CTRPluginFramework
{
    class TextBox
    {
    public:
        TextBox(std::string &text, IntRect box);
        ~TextBox(){}

        // Open the texbox
        void    Open(void);
        // Close the textbox
        void    Close(void);

        // Process Event
        void    ProcessEvent(Event &event);
        // Update
        void    Update(void);
        // Draw
        void    Draw(void);
    private:

        std::string     &_text;
        bool            _isOpen;

        u32             _offset;
    };
}

#endif
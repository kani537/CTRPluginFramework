#ifndef CTRPLUGINFRAMEWORKIMPL_TOUCHKEYSTRING_KEYBOARD_HPP
#define CTRPLUGINFRAMEWORKIMPL_TOUCHKEYSTRING_KEYBOARD_HPP

#include "CTRPluginFrameworkImpl/Graphics/Drawable.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"

#include "CTRPluginFramework/System/Touch.hpp"

#include <string>

namespace CTRPluginFramework
{
    class TouchKeyString : public Drawable
    {
    public:

        // Key with string
        TouchKeyString(std::string content, IntRect ui, bool enabled = true);

        ~TouchKeyString(){}

        // Enabler
        void    Enable(bool enable = true);

        // Draw
        void    Draw(void) override;

        // Update
        void    Update(bool touchIsDown, IntVector touchPos) override;

        // Scrolls
        void    Scroll(float amount);

        // Executer
        // Return -1 if not pressed, 1 otherwise
        int    operator()(void);



    private:
        std::string     _content;
        IntRect         _uiProperties;

        bool            _isPressed;
        bool            _execute;
        bool            _enabled;
        float           _contentLength;
        float           _posY;
    };
}

#endif
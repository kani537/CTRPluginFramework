#ifndef CTRPLUGINFRAMEWORKIMPL_TOUCHKEYSTRING_KEYBOARD_HPP
#define CTRPLUGINFRAMEWORKIMPL_TOUCHKEYSTRING_KEYBOARD_HPP

#include "CTRPluginFrameworkImpl/Graphics/Drawable.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFramework/Graphics/CustomIcon.hpp"

#include "CTRPluginFramework/System/Touch.hpp"

#include <string>

namespace CTRPluginFramework
{
    class TouchKeyString
    {
    public:

        // Key with string
        TouchKeyString(const std::string &content, IntRect ui, bool enabled = true);

		// Key with icon
		TouchKeyString(const CustomIcon& icon, IntRect ui, bool isEnabled);

        ~TouchKeyString(){}

        // Enabler
        void    Enable(bool enable = true);

        // Draw
        void    Draw(void);

        // Update
        void    Update(const bool touchIsDown, const IntVector &touchPos);

        // Scrolls
        void    Scroll(float amount);

        // Executer
        // Return -1 if not pressed, 1 otherwise
        int    operator()(void);

    private:
        std::string     _content;
		CustomIcon		_icon;

        IntRect         _uiProperties;

        bool            _isPressed;
		bool			_isIcon;
        bool            _execute;
        bool            _enabled;
        u16             _posX;
        float           _contentLength;
        float           _posY;

		TouchKeyString(IntRect ui, bool isEnabled);
    };
}

#endif
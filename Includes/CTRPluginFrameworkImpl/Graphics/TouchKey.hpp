#ifndef CTRPLUGINFRAMEWORKIMPL_KEY_KEYBOARD_HPP
#define CTRPLUGINFRAMEWORKIMPL_KEY_KEYBOARD_HPP


#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"

#include "CTRPluginFramework/System/Touch.hpp"


namespace CTRPluginFramework
{
    class TouchKey
    {
    public:
        using IconCallback = int (*)(int, int, bool);

        // Key with char
        TouchKey(int character, IntRect ui, bool enabled = true);
        // Key with Icon
        TouchKey(int value, IconCallback, IntRect ui, bool enabled = true);

        ~TouchKey(){}

        // Enabler
        void    Enable(bool enable = true);

        // Draw
        void    DrawCharacter(const IntRect &rect, char c, Color &color);
        void    Draw(void);

        // Update
        void    Update(bool touchIsDown, IntVector touchPos);

        // Executer
        // Return -1 if not pressed, _character value otherwise
        int    operator()(void);



    private:
        int              _character;
        IconCallback    _icon;
        IntRect         _uiProperties;

        bool            _isPressed;
        bool            _execute;
        bool            _enabled;
    };
}

#endif
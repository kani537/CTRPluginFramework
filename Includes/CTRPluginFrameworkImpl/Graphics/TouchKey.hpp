#ifndef CTRPLUGINFRAMEWORKIMPL_KEY_KEYBOARD_HPP
#define CTRPLUGINFRAMEWORKIMPL_KEY_KEYBOARD_HPP


#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"

#include "CTRPluginFramework/System/Touch.hpp"


namespace CTRPluginFramework
{
    struct KeyContent
    {
        std::string     text;
        float           width;
    };
    class TouchKey
    {
    public:
        using IconCallback = int (*)(int, int, bool);

        // Key with char
        TouchKey(int character, IntRect ui, bool enabled = true);
        // Key with char
        TouchKey(std::string str, IntRect ui, int value = 0x12345678, bool enabled = true);
        // Key with Icon
        TouchKey(int value, IconCallback, IntRect ui, bool enabled = true);

        ~TouchKey(){}

        // Enabler
        void    Enable(bool enable = true);
        void    ChangeContent(std::string content);

        // Draw
        void    DrawCharacter(const IntRect &rect, Color &color);
        void    Draw(void);

        // Update
        void    Update(bool touchIsDown, IntVector touchPos);

        // Executer
        // Return -1 if not pressed, _character value or 0x12345678 otherwise
        int    operator()(std::string &str);



    private:
        int             _character;
        KeyContent      *_content;
        IconCallback    _icon;
        IntRect         _uiProperties;

        bool            _isPressed;
        bool            _execute;
        bool            _enabled;
    };
}

#endif
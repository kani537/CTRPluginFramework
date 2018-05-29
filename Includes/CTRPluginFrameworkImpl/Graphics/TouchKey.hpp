#ifndef CTRPLUGINFRAMEWORKIMPL_KEY_KEYBOARD_HPP
#define CTRPLUGINFRAMEWORKIMPL_KEY_KEYBOARD_HPP

#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/System/Touch.hpp"

#include "CTRPluginFrameworkImpl/Graphics/Drawable.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"

namespace CTRPluginFramework
{
    struct KeyContent
    {
        KeyContent(const std::string &str) :
            text{str}, width{Renderer::GetTextSize(str.c_str())}
        {
        }
        std::string     text;
        float           width;
    };

    class TouchKey : public Drawable
    {
    public:
        using IconCallback = int (*)(int, int, bool);

        TouchKey(TouchKey &&tk);
        // Key with char
        TouchKey(int character, IntRect ui, bool enabled = true);
        // Key with char
        TouchKey(const std::string &str, IntRect ui, int value = 0x12345678, bool enabled = true);
        // Key with Icon
        TouchKey(int value, IconCallback, IntRect ui, bool enabled = true);

        ~TouchKey();

        // Clear
        void    Clear(void);

        // Enabler
        void    Enable(bool enable = true);
        void    ChangeContent(const std::string &content);

        // Draw
        void    DrawCharacter(const IntRect &rect, const Color &color);
        void    Draw(void) override;

        // Update
        void    Update(const bool touchIsDown, const IntVector &touchPos) override;

        // Executer
        // Return -1 if not pressed, _character value or 0x12345678 otherwise
        int    operator()(std::string &str);



    private:
        int             _character{0};
        KeyContent      *_content{nullptr};
        IconCallback    _icon{nullptr};
        IntRect         _uiProperties;

        bool            _isPressed{false};
        bool            _execute{false};
        bool            _enabled{true};
    };
}

#endif
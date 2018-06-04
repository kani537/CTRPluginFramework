#include "CTRPluginFrameworkImpl/Graphics/TouchKeyString.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    TouchKeyString::TouchKeyString(const std::string &content, IntRect ui, bool isEnabled)
    {
        _content = content;
        _uiProperties = ui;
        _posY = _uiProperties.leftTop.y;
        _enabled = isEnabled;

        _isPressed = false;
        _execute = false;

        _contentLength = Renderer::GetTextSize(_content.c_str());

        while ((int)(_contentLength) > ui.size.x)
        {
            if (_content.size() < 1)
                break;
            _content.pop_back();
            _contentLength = Renderer::GetTextSize(_content.c_str());
        }

        u8  *s = reinterpret_cast<u8 *>(const_cast<char *>(_content.c_str()));
        do
        {
            Glyph *glyph = Font::GetGlyph(s);

            if (glyph == nullptr)
                break;

            _glyphs.push_back(glyph);

        } while (*s);

        _posX = ((_uiProperties.size.x - (int)_contentLength) >> 1) + _uiProperties.leftTop.x;
    }

    void    TouchKeyString::Enable(bool isEnabled)
    {
        _enabled = isEnabled;
    }

    void    TouchKeyString::Draw(void)
    {
        // If key is disabled
        if (!_enabled)
            return;

        const auto    &theme = Preferences::Settings.CustomKeyboard;
        const Color &background = _isPressed ? theme.KeyBackgroundPressed : theme.KeyBackground;
        const Color &text = _isPressed ? theme.KeyTextPressed : theme.KeyText;

        int     posX = _posX;
        int     posY = ((_uiProperties.size.y - 16) >> 1) + _uiProperties.leftTop.y;

        // Background
        Renderer::DrawRect(_uiProperties, background);

        // Text
        for (Glyph *glyph : _glyphs)
        {
            posX = Renderer::DrawGlyph(glyph, posX, posY, text);
        }
    }

    void    TouchKeyString::Update(const bool isTouchDown, const IntVector &touchPos)
    {
        if (!_enabled)
            return;

        bool    isTouched = _uiProperties.Contains(touchPos);

        if (_isPressed && !isTouchDown)
        {
            _isPressed = false;
            _execute = true;
        }

        _isPressed = isTouchDown && isTouched;
    }

    void    TouchKeyString::Scroll(float amount)
    {
        _posY += amount;

        int &posY = _uiProperties.leftTop.y;

        posY = _posY;
    }

    int     TouchKeyString::operator()(void)
    {
        if (_enabled && _execute)
        {
            _execute = false;
            return (1);
        }
        return (-1);
    }
}

#include "CTRPluginFrameworkImpl/Graphics/TouchKey.hpp"
#include "ctrulib/util/utf.h"

namespace CTRPluginFramework
{
    TouchKey::TouchKey(int character, IntRect ui, bool isEnabled)
    {
        _character = character;
        _icon = nullptr;
        _uiProperties = ui;
        _enabled = isEnabled;

        _isPressed = false;
        _execute = false;
    }  

    TouchKey::TouchKey(int value, IconCallback icon, IntRect ui, bool isEnabled)
    {
        _character = value;
        _icon = icon;
        _uiProperties = ui;
        _enabled = isEnabled;

        _isPressed = false;
        _execute = false;
    }

    void    TouchKey::Enable(bool isEnabled)
    {
        _enabled = isEnabled;
    }

    void    TouchKey::DrawCharacter(const IntRect &rect, char c, Color &color)
    {
        Glyph *glyph = Font::GetGlyph(c);

        if (glyph == nullptr)
            return;

        float  width = (glyph->xAdvance / 2.f);

        int posX = ((rect.size.x - static_cast<int>(width)) / 2) + rect.leftTop.x;
        int posY = ((rect.size.y - 16) / 2) + rect.leftTop.y;

        Renderer::DrawGlyph(glyph, posX, posY, color);
    }

    void    TouchKey::Draw(void)
    {
        Color    &background = Color::Black;
        Color    &pressed = Color::Silver;
        Color    &character = Color::Blank;
        Color    &characterDis = Color::DimGrey;

        // if key is disabled
        if (!_enabled)
        {
            Renderer::DrawRect(_uiProperties, background);
            if (_icon != nullptr)
            {
                int posX = ((_uiProperties.size.x - 15) / 2) + _uiProperties.leftTop.x;
                int posY = ((_uiProperties.size.y - 15) / 2) + _uiProperties.leftTop.y;
                _icon(posX, posY, false);
            }
            else
                DrawCharacter(_uiProperties, _character, characterDis);
        }
        else if (_isPressed)
        {
            // Background
            Renderer::DrawRect(_uiProperties, pressed);
            // Icon
            if (_icon != nullptr)
            {
                int posX = ((_uiProperties.size.x - 15) / 2) + _uiProperties.leftTop.x;
                int posY = ((_uiProperties.size.y - 15) / 2) + _uiProperties.leftTop.y;
                _icon(posX, posY, true);
            }
            // Character
            else
            {
                DrawCharacter(_uiProperties, _character, character);
            }
        }
        else
        {
            Renderer::DrawRect(_uiProperties, background);
            // Icon
            if (_icon != nullptr)
            {
                int posX = ((_uiProperties.size.x - 15) / 2) + _uiProperties.leftTop.x;
                int posY = ((_uiProperties.size.y - 15) / 2) + _uiProperties.leftTop.y;
                _icon(posX, posY, false);
            }
            // Character
            else
            {
                DrawCharacter(_uiProperties, _character, character);
            }
        }
    }

    void    TouchKey::Update(bool isTouchDown, IntVector touchPos)
    {
        if (!_enabled)
            return;

        bool    isTouched = _uiProperties.Contains(touchPos);

        if (_isPressed && !isTouchDown)
        {            
            _isPressed = false;
            _execute = true;
        } 

        if (isTouchDown && isTouched)
        {
            _isPressed = true;
        }
        else
            _isPressed = false;   
    }

    int      TouchKey::operator()(void)
    {
        if (_enabled && _execute)
        {
            _execute = false;
            return (_character);
        }
        return (-1);
    }
}
#include "CTRPluginFrameworkImpl/Graphics/Key.hpp"
#include "ctrulib/util/utf.h"

namespace CTRPluginFramework
{
    TouchKey::TouchKey(char character, IntRect ui, bool isEnabled)
    {
        _character = character;
        _icon = nullptr;
        _uiProperties = ui;
        _enabled = isEnabled;

        _isPressed = false;
        _execute = false;
    }  

    TouchKey::TouchKey(char value, IconCallback icon, IntRect ui, bool isEnabled)
    {
        _character = value;
        _icon = icon;
        _uiProperties = ui;
        _enabled = isEnabled;

        _isPressed = false;
        _execute = false;
    }

    void    TouchKey::DrawCharacter(const IntRect &rect, char c, Color &color)
    {
        u32             glyphCode;
        u32             index;
        int             units;
        fontGlyphPos_s  glyphPos;
        charWidthInfo_s *cwi;

        units = decode_utf8(&glyphCode, (const u8 *)&c);
        if (units == -1)
            return;
        index = fontGlyphIndexFromCodePoint(glyphCode);
        Renderer::FontCalcGlyphPos(&glyphPos, &cwi, index, 0.5f, 0.5f);

        float  width = glyphPos.xAdvance / 2.f;

        int posX = ((rect.size.x - (int)width) / 2) + rect.leftTop.x;
        int posY = ((rect.size.y - 16) / 2) + rect.leftTop.y;

        Renderer::DrawGlyph(glyphPos, cwi, posX, posY, color, 0.f);
    }

    void    TouchKey::Draw(void)
    {
        static Color    background(0, 0, 0); //Black
        static Color    pressed(192, 192, 192); // silver
        static Color    character(255, 255, 255); // blank
        static Color    characterDis(105, 105, 105); // dimgray

        // if key is disabled
        if (!_enabled)
        {
            Renderer::DrawRect(_uiProperties, background);
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

        if (isTouchDown)
        {
            if (isTouched)
                _isPressed = true;
            else
                _isPressed = false;
        }
        else if (_isPressed)
        {            
            _isPressed = false;
            _execute = true;
        }      
    }

    char    TouchKey::operator()(void)
    {
        if (_enabled && _execute)
        {
            _execute= false;
            return (_character);
        }
        return (-1);
    }
}
#include "CTRPluginFrameworkImpl/Graphics/TouchKey.hpp"
#include "ctrulib/util/utf.h"

namespace CTRPluginFramework
{
    TouchKey::TouchKey(int character, IntRect ui, bool isEnabled)
    {
        _character = character;
        _content = nullptr;
        _icon = nullptr;
        _uiProperties = ui;
        _enabled = isEnabled;

        _isPressed = false;
        _execute = false;
    }

    TouchKey::TouchKey(std::string str, IntRect ui, int value, bool isEnabled)
    {
        _character = value;
        _content = new KeyContent{ str, Renderer::GetTextSize(str.c_str()) };
        _icon = nullptr;
        _uiProperties = ui;
        _enabled = isEnabled;

        _isPressed = false;
        _execute = false;
    }

    TouchKey::TouchKey(int value, IconCallback icon, IntRect ui, bool isEnabled)
    {
        _character = value;
        _content = nullptr;
        _icon = icon;
        _uiProperties = ui;
        _enabled = isEnabled;

        _isPressed = false;
        _execute = false;
    }

    TouchKey::~TouchKey()
    {            
    }

    void TouchKey::Clear(void)
    {
        if (_content != nullptr)
            delete _content;
    }

    void    TouchKey::Enable(bool isEnabled)
    {
        _enabled = isEnabled;
    }

    void    TouchKey::ChangeContent(std::string content)
    {
        if (_content != nullptr)
        {
            _content->text = content;
            _content->width = Renderer::GetTextSize(content.c_str());
        }
    }

    void    TouchKey::DrawCharacter(const IntRect &rect, Color &color)
    {
        // If not a string
        if (_content == nullptr)
        {
            char c = static_cast<char>(_character);
            Glyph *glyph = Font::GetGlyph(c);

            if (glyph == nullptr)
                return;

            int posX = ((rect.size.x - static_cast<int>(glyph->Width())) / 2) + rect.leftTop.x;
            int posY = ((rect.size.y - 16) / 2) + rect.leftTop.y;

            Renderer::DrawGlyph(glyph, posX, posY, color);
        }
        // String
        else
        {
            int posX = ((rect.size.x - _content->width) / 2) + rect.leftTop.x;
            int posY = ((rect.size.y - 16) / 2) + rect.leftTop.y;

            int end = posX + _content->width;

            u8  *str = reinterpret_cast<u8 *>(const_cast<char *>(_content->text.c_str()));
            do
            {
                Glyph *glyph = Font::GetGlyph(str);

                if (glyph == nullptr)
                    break;

                posX = Renderer::DrawGlyph(glyph, posX, posY, color);
            } while (*str);
        }

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
                DrawCharacter(_uiProperties, characterDis);
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
                DrawCharacter(_uiProperties, character);
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
                DrawCharacter(_uiProperties, character);
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

    int      TouchKey::operator()(std::string &str)
    {
        if (_enabled && _execute)
        {
            _execute = false;
            if (_content != nullptr && _character == 0x12345678)
            {
                str += _content->text;
            }
            return (_character);
        }
        return (-1);
    }
}
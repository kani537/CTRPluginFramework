#ifndef CTRPLUGINFRAMEWORK_CHECKEDBUTTON_HPP
#define CTRPLUGINFRAMEWORK_CHECKEDBUTTON_HPP

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/System/Clock.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Drawable.hpp"

#include <vector>
#include <string>

namespace CTRPluginFramework
{
    template <class C, class T, class ...Args>
    class CheckedButton : public Drawable
    {
    public:
        using EventCallback = T (C::*)(Args...);
        using IconCallback = int (*)(int, int);

        CheckedButton(std::string content, C &caller, EventCallback callback, IntRect ui, IconCallback icon = nullptr);
        ~CheckedButton(){}

        // Draw
        void    Draw(void) override;
        // Update
        void    Update(bool isTouchDown, IntVector touchPos) override;
        // Execute
        bool    operator()(void) override;
        // Return if the button is checked or not
        bool    GetState(void) const
        {
            return (_state);
        }

        void    UseSysFont(bool use);

        void    RoundedRatio(u32 ratio)
        {
            if (ratio == 0)
            {
                _lines.clear();
                _useRounded = false;
            }
            else
            {
                _useRounded = true;
                Renderer::ComputeRoundedRectangle(_lines, _uiProperties, ratio, 50);
            }
        }

        Color   borderColor;
        Color   idleColor;
        Color   pressedColor;
        Color   contentColor;
        Color   checkedColor;

    private:
        std::vector<IntLine>    _lines;
        std::string             _content;

        C                       &_caller;
        EventCallback           _callback;
        IntRect                 _uiProperties;
        IconCallback            _icon;

        bool                    _state;
        bool                    _isPressed;
        bool                    _execute;
        bool                    _isWaiting;
        bool                    _useRounded;
        bool                    _useSysfont;
        float                   _textSize;
    };

    #define TCheckedButton CheckedButton<C, T, Args...>

    /*
    ** Select sysfont or not
    ************************/
    template <class C, class T, class... Args>
    void    TCheckedButton::UseSysFont(bool use)
    {
        _useSysfont = use;

        if (use)
        {
            _textSize = Renderer::GetTextSize(_content.c_str());
        }
        else
        {
            _textSize = 6.f * _content.size();
        }

        if (_icon != nullptr)
            _textSize += 18.f;
    }

    //Constructor
    template <class C, class T, class ...Args>
    TCheckedButton::CheckedButton(std::string content, C &caller, EventCallback callback, IntRect ui, IconCallback icon) :
    _content(content), _caller(caller), _callback(callback), _uiProperties(ui), _icon(icon), _useRounded(false), _useSysfont(true), _isPressed(false)
    {
        _state = false;
        _execute = false;
        _isWaiting = false;

        // LightGrey
        borderColor = Color::DarkGrey;
        // Blank
        idleColor   = Color::Gainsboro;
        // Dim Grey
        pressedColor = Color::DimGrey;
        // Black
        contentColor = Color::Black;

        // Limegreen
        checkedColor = Color::LimeGreen;

        _textSize = Renderer::GetTextSize(content.c_str());

        if (icon != nullptr)
            _textSize += 18.f;
    }

    //Draw
    template <class C, class T, class ...Args>
    void    TCheckedButton::Draw(void)
    {      
        Color &fillColor = (_isPressed ? pressedColor : (_state ? checkedColor : idleColor));
        Color &bordColor = borderColor;
        int i;

        if (_useRounded)
        {
            int bMax = _lines.size() - 5;

            for (i = 0; i < bMax; i++)
            {
                IntLine &line = _lines[i];
                // Draw border
                Renderer::_DrawPixel(line.start.x, line.start.y, bordColor);
                Renderer::_DrawPixel(line.end.x, line.end.y, bordColor);

                // Fill line
                IntVector left(line.start);
                IntVector right(line.end);
                left.x++;
                right.x;
                Renderer::DrawLine(left, right, fillColor);
            }

            for (; i < _lines.size() - 1; i++)
            {
                IntLine &line = _lines[i];

                Renderer::DrawLine(line.start.x, line.start.y, line.end.x, bordColor, line.end.y);
            }

            IntLine &line = _lines[i];
            Renderer::DrawLine(line.start.x, line.start.y, line.end.x, fillColor, line.end.y);
        }
        else
        {
            Renderer::DrawRect(_uiProperties, fillColor);
            Renderer::DrawRect(_uiProperties, bordColor, false);
        }


        int posX = _uiProperties.leftTop.x;
        int posY = _uiProperties.leftTop.y;

        int height = _uiProperties.size.y;
        int width = _uiProperties.size.x;
        int limit = posX + width;

        if (_useSysfont)
            posY += (height - 16) / 2;
        else
            posY += (height - 10) / 2;

        int posXI = 0;

        int x = (width - _textSize) / 2;
        if (x > 0)
            posX += x;

        if (_icon != nullptr)
            posX = _icon(posX, posY) + 3;

        if (_useSysfont)
        {
            Renderer::DrawSysString(_content.c_str(), posX, posY, limit, contentColor);
        }
        else
            Renderer::DrawString((char *)_content.c_str(), posX, posY, contentColor);
    }

    // Update
    template <class C, class T, class ...Args>
    void    TCheckedButton::Update(bool isTouchDown, IntVector touchPos)
    {
        if (!isTouchDown && _isWaiting)
        {
            _state = !_state;
            _execute = true;
            _isWaiting = false;
        }

        if (isTouchDown)
        {            
            _isPressed = _uiProperties.Contains(touchPos);
            if (_isPressed)
                _isWaiting = true;
            else if (_isWaiting)
                _isWaiting = false;
        }
        else
            _isPressed = false;

        
    }

    // Operator
    template <class C, class T, class ...Args>
    bool    TCheckedButton::operator()(void)
    {
        if (_execute)
        {
            if (_callback != nullptr)
                (_caller.*_callback)();
            _execute = false;
            return (true);
        }
        return (false);
    }
}

#endif
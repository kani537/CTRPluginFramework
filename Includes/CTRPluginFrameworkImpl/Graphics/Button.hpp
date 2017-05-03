#ifndef CTRPLUGINFRAMEWORK_BUTTON_H
#define CTRPLUGINFRAMEWORK_BUTTON_H


#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/System/Touch.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"

#include <string>
#include <vector>


namespace CTRPluginFramework
{
    // class C -> type of the caller object
    // class T -> return type of the callback
    // Args -> potential args for the callback
    template <class C, class T, class... Args>
    class Button
    {
        
    public:
        using EventCallback = T (C::*)(Args...);
        using IconCallback = int (*)(int, int);

        Button(std::string content, C &caller, T(C::*callback)(Args...), IntRect rect, IconCallback icon = nullptr) :
        _caller(caller), 
        _callback(callback), 
        _content(content), 
        _uiProperty(rect),
        _icon(icon),
        _isReady(true),
        _useSysfont(true),
        _useRounded(false),
        IsEnabled(true),
        IsLocked(false)
        {
            borderColor = Color::DarkGrey;
            disabledColor = Color::Grey;
            idleColor   = Color::Gainsboro;
            pressedColor = Color::DimGrey;
            contentColor = Color::Black;

            _textSize = Renderer::GetTextSize(content.c_str());
            if (icon != nullptr)
                _textSize += 18.f;

            _isPressed = false;
            _execute = false;
        }
        ~Button(){};

        bool    operator ()(Args&... args)
        {
            if (!IsLocked && IsEnabled && _execute)
            {
                if (_callback != nullptr)
                    (_caller.*(_callback))(args...);
                _execute = false;
                return (true);
            }
            return (false);
        }

        void    Draw(void);
        void    Update(bool isTouchDown, IntVector touchPos);
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
                Renderer::ComputeRoundedRectangle(_lines, _uiProperty, ratio, 50);
            }
        }

        Color           borderColor;
        Color           disabledColor;
        Color           idleColor;
        Color           pressedColor;
        Color           contentColor;
        bool            IsEnabled;
        bool            IsLocked;

    private:

        


        C                       &_caller;
        EventCallback           _callback;
        IconCallback            _icon;
        std::vector<IntLine>    _lines;

        std::string             _content;
        IntRect                 _uiProperty;
        float                   _textSize;        
        bool                    _isPressed;
        bool                    _execute;
        bool                    _isReady;
        bool                    _useSysfont;
        bool                    _useRounded;
    };

    /*
    ** Select sysfont or not
    ************************/
    template <class C, class T, class... Args>
    void    Button<C, T, Args...>::UseSysFont(bool use)
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

    /*
    ** Draw
    *********/

    template <class C, class T, class... Args>
    void    Button<C, T, Args...>::Draw(void)
    {
        //Renderer::RoundedRectangle(_uiProperty, 7, 50, borderColor, true, isPressed ? pressedColor : idleColor);
        if (!IsEnabled)
            return;

        Color &fillColor = IsLocked ? disabledColor : (_isPressed ? pressedColor : idleColor);
        Color &bordColor = IsLocked ? disabledColor : borderColor;
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
            Renderer::DrawRect(_uiProperty, fillColor);
            Renderer::DrawRect(_uiProperty, bordColor, false);
        }
        

        int posX = _uiProperty.leftTop.x;
        int posY = _uiProperty.leftTop.y;

        int height = _uiProperty.size.y;
        int width = _uiProperty.size.x;
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

    /*
    ** Update
    ***********/
    template <class C, class T, class... Args>
    void    Button<C,T, Args...>::Update(bool isTouchDown, IntVector touchPos)
    {
        if (!IsEnabled || IsLocked)
            return;

        // Check if is pressed
        _isPressed = false;

        if (isTouchDown)
        {
            _isPressed = _uiProperty.Contains(touchPos);
            if (!_isReady && !_isPressed)
                _isReady = true;
        }

        // If is ready, then execute the function
        if (_isPressed && _isReady)
        {
            _isReady = false;
        }
        if (!isTouchDown && !_isReady)
        {
            _isReady = true;
            _execute = true;
        }
    }
}

#endif
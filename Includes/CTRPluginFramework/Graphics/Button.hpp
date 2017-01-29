#ifndef CTRPLUGINFRAMEWORK_BUTTON_H
#define CTRPLUGINFRAMEWORK_BUTTON_H

#include "CTRPluginFramework/Line.hpp"
#include "CTRPluginFramework/Rect.hpp"
#include "CTRPluginFramework/Screen.hpp"
#include "CTRPluginFramework/Touch.hpp"
#include "CTRPluginFramework/Graphics/Renderer.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"

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
        _icon(icon)
        {
            // Black
            borderColor = Color(165, 165, 165);
            // Blank
            idleColor   = Color(250, 250, 250);
            // Dim Grey
            pressedColor = Color(105, 105, 105);
            // Black
            contentColor = Color();

            _textSize = Renderer::GetTextSize(content.c_str());
            if (icon != nullptr)
                _textSize += 18.f;

            Renderer::ComputeRoundedRectangle(_lines, _uiProperty, 7, 50);
            _isPressed = false;
            _execute = false;
        }
        ~Button(){};

        virtual bool    operator ()(Args&... args)
        {
            if (_execute)
            {
                (_caller.*(_callback))(args...);
                _execute = false;
                return (true);
            }
            return (false);
        }

        virtual void    Draw(void);
        virtual void    Update(bool isTouchDown, IntVector touchPos);

        Color           borderColor;
        Color           idleColor;
        Color           pressedColor;
        Color           contentColor;
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
    };

    /*
    ** Draw
    *********/

    template <class C, class T, class... Args>
    void    Button<C, T, Args...>::Draw(void)
    {
        //Renderer::RoundedRectangle(_uiProperty, 7, 50, borderColor, true, isPressed ? pressedColor : idleColor);
        
        Color &fillColor = _isPressed ? pressedColor : idleColor;
        int bMax = _lines.size() - 5;
        int i;

        for (i = 0; i < bMax; i++)
        {
            IntLine &line = _lines[i];
            // Draw border
            Renderer::_DrawPixel(line.start.x, line.start.y, borderColor);
            Renderer::_DrawPixel(line.end.x, line.end.y, borderColor);

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

            Renderer::DrawLine(line.start.x, line.start.y, line.end.x, borderColor, line.end.y);
        }

        IntLine &line = _lines[i];
        Renderer::DrawLine(line.start.x, line.start.y, line.end.x, fillColor, line.end.y);

        int posX = _uiProperty.leftTop.x;
        int posY = _uiProperty.leftTop.y;

        int height = _uiProperty.size.y;
        int width = _uiProperty.size.x;
        int limit = posX + width;

        posY += (height - 16) / 2;

        int posXI = 0;

        
        //posY += (height - 10) / 2;
        int x = (width - _textSize) / 2;
        if (x > 0)
            posX += x;

        if (_icon != nullptr)
        {
            posX = _icon(posX, posY) + 3;
        }
        //Renderer::DrawString((char *)_content.c_str(), posX, posY, contentColor);
        Renderer::DrawSysString(_content.c_str(), posX, posY, limit, contentColor);        
    }

    /*
    ** Update
    ***********/
    template <class C, class T, class... Args>
    void    Button<C,T, Args...>::Update(bool isTouchDown, IntVector touchPos)
    {
        static bool isReady = true;

        // Check if is pressed
        _isPressed = false;

        if (isTouchDown)
        {
            _isPressed = _uiProperty.Contains(touchPos); 
        }

        // If is ready, then execute the function
        if (_isPressed && isReady)
        {
            isReady = false;
        }
        if (!_isPressed && !isReady)
        {
            isReady = true;
            _execute = true;
        }
    }
}

#endif
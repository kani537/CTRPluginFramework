#ifndef CTRPLUGINFRAMEWORK_BUTTON_H
#define CTRPLUGINFRAMEWORK_BUTTON_H

#include "CTRPluginFramework.hpp"
#include "Renderer.hpp"
#include "Touch.hpp"
//#include "Vector.h"
#include "Rect.hpp"
#include <string>

namespace CTRPluginFramework
{
    template <class C, class T, class... Args>
    class Button
    {
        
    public:
        typedef T(C::*EventCallback)(Args...);
        Button(std::string content, C &caller, T(C::*callback)(Args...), IntRect rect) :
        _caller(caller), _callback(callback), _content(content), _uiProperty(rect)
        {
            // Black
            borderColor = Color(1, 1, 1);
            // Blank
            idleColor   = Color(250, 250, 250);
            // Dim Grey
            pressedColor = Color(105, 105, 105);
            // Black
            contentColor = Color();

            _textSize = Renderer::GetTextSize(content.c_str());
        }
        ~Button(){};

        virtual bool    operator ()(Args&... args)
        {
            static bool isReady = true;

            // Check if is pressed
            bool isPressed = _Update();
            // Draw button
            _Draw(isPressed);

            // If is ready, then execute the function
            if (isPressed && isReady)
            {
                if (_callback)
                    (_caller.*_callback)(args...);
                isReady = false;
                return (true);
            }
            if (!isPressed && !isReady)
            {
                isReady = true;
            }
            return (false);
        }

        Color           borderColor;
        Color           idleColor;
        Color           pressedColor;
        Color           contentColor;
    private:

        virtual bool    _Update(void)
        {
            if (!Touch::IsDown())
                return (false);

            IntVector touch = IntVector(Touch::GetPosition());
            return (_uiProperty.Contains(touch));
        }

        virtual void    _Draw(bool isPressed)
        {
            Renderer::RoundedRectangle(_uiProperty, 7, 50, borderColor, true, isPressed ? pressedColor : idleColor);
            
            int posX = _uiProperty._leftTopCorner.x;
            int posY = _uiProperty._leftTopCorner.y;

            int height = _uiProperty._size.y;
            int width = _uiProperty._size.x;
            int limit = posX + width;

            posY += (height - 16) / 2;
            int x = (width - _textSize) / 2;
            if (x > 0)
                posX += x;

            Renderer::DrawSysString(_content.c_str(), posX, posY, limit, contentColor);        
        }


        C               &_caller;
        EventCallback   _callback;
        std::string     _content;
        IntRect         _uiProperty;
        float           _textSize;

    };

}

#endif
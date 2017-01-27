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
    template <class C, class T, class... Args>
    class Button
    {
        
    public:
        using EventCallback = T (C::*)(Args...);
        //typedef T(C::*EventCallback)(Args...);
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

            Renderer::ComputeRoundedRectangle(_lines, _uiProperty, 7, 50);
        }
        ~Button(){};

        virtual bool    operator ()(bool isTouchDown, IntVector touchPos, Args&... args)
        {
            static bool isReady = true;

            // Check if is pressed
            bool isPressed = false;

            if (isTouchDown)
            {
                isPressed = _uiProperty.Contains(touchPos);
            }

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

        virtual void    _Draw(bool isPressed)
        {
            //Renderer::RoundedRectangle(_uiProperty, 7, 50, borderColor, true, isPressed ? pressedColor : idleColor);
            
            Color &fillColor = isPressed ? pressedColor : idleColor;
            int bMax = _lines.size() - 5;
            int i;

            for (i = 0; i < bMax; i++)
            {
                IntLine &line = _lines[i];
                // Draw border
                Renderer::_DrawPixel(line.start.x, line.start.y, borderColor);
                Renderer::_DrawPixel(line.end.x, line.end.y, borderColor);
               /* if (i == 0)
                {
                    char buf[40];
                    sprintf(buf, "[%d, %d] - [%d, %d]", line.start.x, line.start.y, line.end.x, line.end.y);
                    int posY = 50;
                    Color blank(255, 255, 255);
                    Renderer::DrawString(buf, 150, posY, blank);
                }*/
                // Fill line
                IntVector left(line.start);
                IntVector right(line.end);
                left.x++;
                right.x--;
                Renderer::DrawLine(left, right, fillColor);
            }

            for (; i < _lines.size() - 1; i++)
            {
                IntLine &line = _lines[i];

                Renderer::DrawLine(line.start.x, line.start.y, line.end.x, borderColor, line.end.y);
            }

            IntLine &line = _lines[i];
            Renderer::DrawLine(line.start.x, line.start.y, line.end.x, fillColor, line.end.y);

            int posX = _uiProperty._leftTopCorner.x;
            int posY = _uiProperty._leftTopCorner.y;

            int height = _uiProperty._size.y;
            int width = _uiProperty._size.x;
            int limit = posX + width;

            posY += (height - 16) / 2;
            //posY += (height - 10) / 2;
            int x = (width - _textSize) / 2;
            if (x > 0)
                posX += x;
            //Renderer::DrawString((char *)_content.c_str(), posX, posY, contentColor);
            Renderer::DrawSysString(_content.c_str(), posX, posY, limit, contentColor);        
        }


        C               &_caller;
        EventCallback   _callback;
        std::string     _content;
        IntRect         _uiProperty;
        float           _textSize;
        std::vector<IntLine> _lines;

    };

}

#endif
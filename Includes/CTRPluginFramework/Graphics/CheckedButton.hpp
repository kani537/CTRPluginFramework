#ifndef CTRPLUGINFRAMEWORK_CHECKEDBUTTON_HPP
#define CTRPLUGINFRAMEWORK_CHECKEDBUTTON_HPP

#include "CTRPluginFramework/Line.hpp"
#include "CTRPluginFramework/Rect.hpp"
#include "CTRPluginFramework/Screen.hpp"
#include "CTRPluginFramework/Touch.hpp"
#include "CTRPluginFramework/Graphics/Renderer.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/Clock.hpp"
#include "CTRPluginFramework/Graphics/Icon.hpp"

#include <vector>
#include <string>

namespace CTRPluginFramework
{
    template <class C, class T, class ...Args>
    class CheckedButton
    {
    public:
        using EventCallback = T (C::*)(Args...);
        using IconCallback = int (*)(int, int);

        CheckedButton(std::string content, C &caller, EventCallback callback, IntRect ui, IconCallback icon = nullptr);
        ~CheckedButton(){}

        // Draw
        void    Draw(void);
        // Update
        void    Update(bool isTouchDown, IntVector touchPos);
        // Execute
        bool    operator()(Args ...args);

        Color   borderColor;
        Color   idleColor;  
        Color   pressedColor;
        Color   contentColor;

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
        float                   _textSize;
    };

    #define TCheckedButton CheckedButton<C, T, Args...>

    //Constructor
    template <class C, class T, class ...Args>
    TCheckedButton::CheckedButton(std::string content, C &caller, EventCallback callback, IntRect ui, IconCallback icon) :
    _content(content), _caller(caller), _callback(callback), _uiProperties(ui), _icon(icon)
    {
        _state = false;
        _execute = false;

        // LightGrey
        borderColor = Color(165, 165, 165);
        // Blank
        idleColor   = Color(250, 250, 250);
        // Dim Grey
        pressedColor = Color(105, 105, 105);
        // Black
        contentColor = Color();

        _textSize = Renderer::GetTextSize(content.c_str());
        _textSize += 18.f;

        if (icon != nullptr)
            _textSize += 8.f;

        Renderer::ComputeRoundedRectangle(_lines, _uiProperties, 7, 50);
    }

    //Draw
    template <class C, class T, class ...Args>
    void    TCheckedButton::Draw(void)
    {      
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

        int posX = _uiProperties._leftTopCorner.x;
        int posY = _uiProperties._leftTopCorner.y;

        int height = _uiProperties._size.y;
        int width = _uiProperties._size.x;
        int limit = posX + width;

        posY += (height - 16) / 2;

        int posXI = 0;

        
        int x = (width - _textSize) / 2;
        if (x > 0)
            posX += x;

        posX = Icon::DrawCheckBox(posX, posY, _state);
        if (_icon != nullptr)
            posX = _icon(posX, posY);
        Renderer::DrawSysString(_content.c_str(), posX, posY, limit, contentColor);        
    }

    // Update
    template <class C, class T, class ...Args>
    void    TCheckedButton::Update(bool isTouchDown, IntVector touchPos)
    {

        if (isTouchDown && _uiProperties.Contains(touchPos))
        {
            _execute = true;
            if (_isPressed != isTouchDown)
                _state = !_state;
            _isPressed = true;
        }
        else
            _isPressed = false;
    }

    // Operator
    template <class C, class T, class ...Args>
    bool    TCheckedButton::operator()(Args ...args)
    {
        if (_execute)
        {
            (_caller.*_callback)(args...);
            _execute = false;
            return (true);
        }
        return (false);
    }
}

#endif
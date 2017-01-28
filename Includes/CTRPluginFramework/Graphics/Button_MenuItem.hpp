#ifndef CTRPLUGINFRAMEWORK_BUTTON_MENUITEM_H
#define CTRPLUGINFRAMEWORK_BUTTON_MENUITEM_H

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
    class MenuItem;
    // class C -> type of the caller object
    // class T -> return type of the callback
    // Args -> potential args for the callback
    template <class T, class... Args>
    class Button<MenuItem, T, Args...>
    {
        
    public:
        using EventCallback = T (MenuItem::*)(Args...);
        using IconCallback = int (*)(int, int, bool);

        Button(MenuItem &caller, T(MenuItem::*callback)(Args...), IntRect rect, IconCallback icon) :
        _caller(caller), 
        _callback(callback),
        _uiProperty(rect),
        _icon(icon)
        {
            _isPressed = false;
            _execute = false;
        }
        ~Button(){};

        virtual bool    operator ()(Args&... args)
        {
            if (_execute)
            {
                //(_caller.*(_callback))(args...);
                _execute = false;
                return (true);
            }
            return (false);
        }

        virtual void    Draw(void);
        virtual void    Update(bool isTouchDown, IntVector touchPos);

    private:       


        MenuItem                &_caller;
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

    template <class T, class... Args>
    void    Button<MenuItem, T, Args...>::Draw(void)
    {
        int posX = _uiProperty._leftTopCorner.x;
        int posY = _uiProperty._leftTopCorner.y;

        if (_icon != nullptr)
        {
            _icon(posX, posY, _isPressed);
        }      
    }

    /*
    ** Update
    ***********/
    template <class T, class... Args>
    void    Button<MenuItem, T, Args...>::Update(bool isTouchDown, IntVector touchPos)
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
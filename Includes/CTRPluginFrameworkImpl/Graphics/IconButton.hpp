#ifndef CTRPLUGINFRAMEWORKIMPL_ICONBUTTON_HPP
#define CTRPLUGINFRAMEWORKIMPL_ICONBUTTON_HPP

#include "CTRPluginFrameworkImpl/Graphics/Drawable.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/System/Rect.hpp"
#include "CTRPluginFramework/System/Touch.hpp"
#include "CTRPluginFramework/System/Clock.hpp"


namespace CTRPluginFramework
{
    template <class C, class T, class ...Args>
    class IconButton : public Drawable
    {
    public:
        using EventCallback = T (C::*)(Args...);
        using IconCallback = int (*)(int, int, bool);

        // Create the object
        IconButton(C &caller, EventCallback callback, IntRect ui, IconCallback icon, bool enabled = true);
        virtual ~IconButton(){}

        // Change the state of the object
        void    SetState(bool state);

        // Draw
        void    Draw(void) override;

        // Update
        void    Update(bool touchIsDown, IntVector touchPos) override;

        // Executer
        virtual bool    operator()(Args ...args);

        // Enabler
        void    Enable(bool enable = true);

    protected:
        C               &_caller;
        EventCallback   _callback;
        IconCallback    _icon;
        IntRect         _uiProperties;

        bool            _state;
        bool            _execute;
        bool            _enabled;
    };

    #define TIconButton IconButton<C, T, Args...>

    // Constructor
    template <class C, class T, class ...Args>
    TIconButton::IconButton(C &caller, EventCallback callback, IntRect ui, IconCallback icon, bool enabled) :
    _caller(caller), _callback(callback), _uiProperties(ui), _icon(icon), _state(false), _execute(false), _enabled(enabled)
    {

    }

    // State
    template <class C, class T, class ...Args>
    void    TIconButton::SetState(bool isActivated)
    {
        _state = isActivated;
    }

    // Draw
    template <class C, class T, class ...Args>
    void    TIconButton::Draw(void)
    {
        if (_enabled)
            _icon(_uiProperties.leftTop.x, _uiProperties.leftTop.y, _state);
    }

    // Update
    template <class C, class T, class ...Args>
    void    TIconButton::Update(bool isTouchdown, IntVector touchPos)
    {
        if (!_enabled)
            return;
        if (!_state && isTouchdown && _uiProperties.Contains(touchPos))
            _state = true;

        if (_state && isTouchdown && !_uiProperties.Contains(touchPos))
            _state = false;
        
        if (_state && !isTouchdown)
        {
            _execute = true;
            _state = false;
        }
    }

    // Executer
    template <class C, class T, class ...Args>
    bool    TIconButton::operator()(Args ...args)
    {
        if (_enabled && _execute)
        {
            if (_callback != nullptr)
                (_caller.*_callback)(args...);
            _execute = false;
            return (true);
        }
        return (false);
    }

    // Enabler
    template <class C, class T, class ...Args>
    void    TIconButton::Enable(bool enable)
    {
        _enabled = enable;
    }
}

#endif
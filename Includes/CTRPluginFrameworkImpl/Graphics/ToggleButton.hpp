#ifndef CTRPLUGINFRAMEWORK_TOGGLEBUTTON_HPP
#define CTRPLUGINFRAMEWORK_TOGGLEBUTTON_HPP

#include "CTRPluginFrameworkImpl/Graphics/Drawable.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"

#include "CTRPluginFramework/System/Touch.hpp"
#include "CTRPluginFramework/System/Clock.hpp"


namespace CTRPluginFramework
{
    template <class C, class T, class ...Args>
    class ToggleButton : public Drawable
    {
    public:
        using EventCallback = T (C::*)(Args...);
        using IconCallback = int (*)(int, int, bool);

        // Create the object
        ToggleButton(C &caller, EventCallback callback, IntRect ui, IconCallback icon, bool enabled = true);
        ~ToggleButton(){}

        // Change the state of the object
        void    SetState(bool state);

        // Draw
        void    Draw(void) override;

        // Update
        void    Update(const bool touchIsDown, const IntVector &touchPos) override;

        // Executer
        bool    operator()(void) override;

        // Enabler
        void    Enable(bool enable = true);

    private:
        C               &_caller;
        EventCallback   _callback;
        IconCallback    _icon;
        IntRect         _uiProperties;

        bool            _state;
        bool            _execute;
        bool            _enabled;
        Clock           _clock;
    };

    #define TToggleButton ToggleButton<C, T, Args...>

    // Constructor
    template <class C, class T, class ...Args>
    TToggleButton::ToggleButton(C &caller, EventCallback callback, IntRect ui, IconCallback icon, bool enabled) :
    _caller(caller), _callback(callback), _uiProperties(ui), _icon(icon), _state(false), _execute(false), _enabled(enabled)
    {

    }

    // State
    template <class C, class T, class ...Args>
    void    TToggleButton::SetState(bool isActivated)
    {
        _state = isActivated;
    }

    // Draw
    template <class C, class T, class ...Args>
    void    TToggleButton::Draw(void)
    {
        if (_enabled)
            _icon(_uiProperties.leftTop.x, _uiProperties.leftTop.y, _state);
    }

    // Update
    template <class C, class T, class ...Args>
    void    TToggleButton::Update(const bool isTouchdown, const IntVector &touchPos)
    {
        if (!_enabled)
            return;
        if (_clock.HasTimePassed(Seconds(0.5f)) && isTouchdown && _uiProperties.Contains(touchPos))
        {
            _state = !_state;
            _execute = true;
            _clock.Restart();
        }
    }

    // Executer
    template <class C, class T, class ...Args>
    bool    TToggleButton::operator()(void)
    {
        if (_enabled && _execute)
        {
            if (_callback != nullptr)
                (_caller.*_callback)();
            _execute = false;
            return (true);
        }
        return (false);
    }

    // Enabler
    template <class C, class T, class ...Args>
    void    TToggleButton::Enable(bool enable)
    {
        _enabled = enable;
    }
}

#endif
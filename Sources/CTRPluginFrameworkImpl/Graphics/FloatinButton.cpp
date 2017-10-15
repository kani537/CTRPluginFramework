#include "CTRPluginFrameworkImpl/Graphics/FloatingButton.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"

namespace CTRPluginFramework
{
    FloatingButton::FloatingButton(const IntRect &pos, const IconCallback icon) :
        _icon(icon), _box(pos), _lastTouch{ 0,0 },
        _pressed(false), _enabled(false), _moving(false)
    {        
    }

    FloatingButton::~FloatingButton()
    {
    }

    void    FloatingButton::Draw(void)
    {
        if (_icon != nullptr)
        {
            _icon(_box.leftTop.x, _box.leftTop.y);
        }
    }

    bool    FloatingButton::operator()()
    {
        bool ret = _enabled;

        _enabled = false;
        return (ret);
    }
#define ABS(x) ((x) >= 0 ? (x) : (-x))
#define IsDifferentByAtLeast(old, new, val) (old != new && (ABS(new - old) >= val))

    void    FloatingButton::Update(const bool isTouchDown, const IntVector &touchPos)
    {
        IntVector &currentPos = _box.leftTop;
        const IntVector &size = _box.size;

        // If the user is moving
        if (isTouchDown && _moving)
        {
            // Adjust the position
            currentPos.x = std::min(std::max(touchPos.x - size.x / 2, 0), 319 - size.x);
            currentPos.y = std::min(std::max(touchPos.y - size.y / 2, 0), 239 - size.y);
            return;
        }

        if (isTouchDown)
        {
            if (_box.Contains(touchPos))
            {
                // If user just pressed the button, save the current state
                if (!_pressed)
                {
                    _lastTouch = touchPos;
                    _pressed = true;
                    return;
                }

                if (!_moving)
                {
                    if (IsDifferentByAtLeast(_lastTouch.x, touchPos.x, 5)
                        || IsDifferentByAtLeast(_lastTouch.y, touchPos.y, 5))
                    {
                        _moving = true;
                    }
                }
            }
            else
            {
                _pressed = false;
                _moving = false;
            }
        }
        else
        {
            if (_pressed && !_moving)
            {
                _enabled = true;
            }
            _pressed = false;
            _moving = false;
        }
    }
}

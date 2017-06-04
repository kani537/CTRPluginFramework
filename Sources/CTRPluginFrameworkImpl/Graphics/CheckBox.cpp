#include "CTRPluginFrameworkImpl/Graphics/CheckBox.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"

namespace CTRPluginFramework
{
    CheckBox::CheckBox(int posX, int posY) :
    IconButton<CTRPluginFramework::CheckBox, void>(*this, nullptr, IntRect(posX, posY, 20, 20), Icon::DrawCheckBox),
    _isWaiting(false),
    _isPressed(false)
    {
    }

    CheckBox::~CheckBox()
    {
    }

    void CheckBox::Update(bool isTouchDown, IntVector touchPos)
    {
        if (!_enabled)
            return;

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

    bool CheckBox::IsChecked(void) const
    {
        return (_state);
    }
}

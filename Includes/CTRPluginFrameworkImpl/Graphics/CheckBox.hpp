#ifndef CTRPLUGINFRAMEWORKIMPL_CHECKBOX_HPP
#define CTRPLUGINFRAMEWORKIMPL_CHECKBOX_HPP

#include "CTRPluginFrameworkImpl/Graphics/IconButton.hpp"

namespace CTRPluginFramework
{
    class CheckBox;
    class CheckBox : public IconButton<CheckBox, void>
    {
    public:
        CheckBox(int posX, int posY);
        ~CheckBox(void);

        void    Update(const bool isTouchDown, const IntVector &touchPos) override;
        bool    IsChecked(void) const;

    protected:
        bool    _isWaiting;
        bool    _isPressed;
    };
}

#endif

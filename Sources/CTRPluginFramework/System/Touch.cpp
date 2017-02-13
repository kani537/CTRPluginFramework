#include "types.h"
#include "ctrulib/services/hid.h"

#include "CTRPluginFramework/System/Controller.hpp"
#include "CTRPluginFramework/System/Touch.hpp"

namespace CTRPluginFramework
{
    bool        Touch::IsDown(void)
    {
        return (Controller::IsKeyDown(Key::Touchpad));
    }

    UIntVector  Touch::GetPosition(void)
    {
        touchPosition   tp;

        hidTouchRead(&tp);
        return (UIntVector(tp.px, tp.py));
    }
}
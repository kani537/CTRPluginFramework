#ifndef CTRPLUGINFRAMEWORK_TOUCH_HPP
#define CTRPLUGINFRAMEWORK_TOUCH_HPP

#include "CTRPluginFramework/Vector.h"

namespace CTRPluginFramework
{
    class Touch
    {
        static bool         IsDown(void);
        static UIntVector   GetPosition(void);
    };
}

#endif
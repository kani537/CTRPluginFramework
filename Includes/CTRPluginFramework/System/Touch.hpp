#ifndef CTRPLUGINFRAMEWORK_TOUCH_HPP
#define CTRPLUGINFRAMEWORK_TOUCH_HPP

#include "CTRPluginFramework/Vector.hpp"
#include "ctrulib/services/hid.h"

namespace CTRPluginFramework
{


    class Touch
    {

    public:
        static bool         IsDown(void);
        static UIntVector   GetPosition(void);
    };
}

#endif
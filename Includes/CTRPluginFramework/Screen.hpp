#ifndef CTRPLUGINFRAMEWORK_SCREEN_HPP
#define CTRPLUGINFRAMEWORK_SCREEN_HPP

#include "types.h"

namespace CTRPluginFramework
{
    struct  LCDFamebufferSetup
    {
        u16     width;
        u16     height;
        u32     leftScreen1;
        u32     leftScreen2;
        u32     format;
        u32     index;
        u32     disco;
        u32     stride;
        u32     rightScreen1;
        u32     rightScreen2;
    };

    class Screen
    {
    public:

    private:

    };
}

#endif
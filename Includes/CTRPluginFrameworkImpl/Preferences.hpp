#ifndef CTRPPLUGINFRAMEWORK_PREFERENCES_HPP
#define CTRPPLUGINFRAMEWORK_PREFERENCES_HPP

#include "types.h"
#include "CTRPluginFrameworkImpl/Graphics/BMPImage.hpp"

namespace CTRPluginFramework
{
    class Preferences
    {
    public:
        static BMPImage     *topBackgroundImage;
        static BMPImage     *bottomBackgroundImage;

        static u32          MenuHotkeys;
        static bool         InjectBOnMenuClose;
        static bool         DrawTouchCursor;
        static bool         EcoMemoryMode;
    private:
        friend void     ThreadInit(void *arg);
        static void     Initialize(void);
    };
}

#endif
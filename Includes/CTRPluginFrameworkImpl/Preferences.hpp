#ifndef CTRPPLUGINFRAMEWORK_PREFERENCES_HPP
#define CTRPPLUGINFRAMEWORK_PREFERENCES_HPP

#include "CTRPluginFrameworkImpl/Graphics/BMPImage.hpp"

namespace CTRPluginFramework
{
    class Preferences
    {
    public:
        static BMPImage     *topBackgroundImage;
        static BMPImage     *bottomBackgroundImage;

        static bool         InjectBOnMenuClose;
        static bool         DrawTouchCursor;
    private:
        friend void     ThreadInit(void *arg);
        static void     Initialize(void);

    };
}

#endif
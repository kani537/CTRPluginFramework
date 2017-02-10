#ifndef CTRPPLUGINFRAMEWORK_PREFERENCES_HPP
#define CTRPPLUGINFRAMEWORK_PREFERENCES_HPP

#include "CTRPluginFramework/Graphics/BMPImage.hpp"

namespace CTRPluginFramework
{
    class Preferences
    {
    public:
        static BMPImage     *topBackgroundImage;
        static BMPImage     *bottomBackgroundImage;
    private:
        friend void     ThreadInit(void *arg);
        static void     Initialize(void);

    };
}

#endif
#include "CTRPluginFramework/Preferences.hpp"

namespace CTRPluginFramework
{
    BMPImage    *Preferences::topBackgroundImage = nullptr;
    BMPImage    *Preferences::bottomBackgroundImage = nullptr;

    void    Preferences::Initialize(void)
    {
        topBackgroundImage = new BMPImage("TopBackground.bmp");
        bottomBackgroundImage = new BMPImage("BottomBackground.bmp");
    }
}
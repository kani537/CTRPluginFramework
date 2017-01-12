#include "CTRPluginFramework.hpp"
#include "3DS.h"

namespace CTRPluginFramework
{
    void    Sleep(Time sleepTime)
    {
        if (sleepTime > Time::Zero)
            svcSleepThread(sleepTime.AsMicroseconds() * 1000);
    }
}
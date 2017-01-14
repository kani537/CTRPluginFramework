#include "CTRPluginFramework.hpp"
#include "3DS.h"

namespace CTRPluginFramework
{
    #define TICKS_PER_SEC 268123480

    static Time GetCurrentTime(void)
    {
        return (Seconds(static_cast<float>(svcGetSystemTick())/TICKS_PER_SEC));
    }

    Clock::Clock(void) : _startTime(GetCurrentTime())
    {

    }

    Time    Clock::GetElapsedTime(void) const
    {
        return (GetCurrentTime() - _startTime);
    }

    bool    Clock::HasTimePassed(Time time) const
    {
        return (GetElapsedTime() >=  time);
    }

    Time    Clock::Restart(void)
    {
        Time now = GetCurrentTime();

        Time ret = now - _startTime;

        _startTime = now;
        return (ret);
    }
}
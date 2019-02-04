#include "types.h"
#include "ctrulib/svc.h"

#include "CTRPluginFramework/System/Time.hpp"
#include "CTRPluginFramework/System/Clock.hpp"

namespace CTRPluginFramework
{
    #define TICKS_PER_SEC       268123480
    #define SYSCLOCK_SOC       (16756991)
    #define SYSCLOCK_ARM9      (SYSCLOCK_SOC * 8)
    #define SYSCLOCK_ARM11     (SYSCLOCK_ARM9 * 2)
    #define SYSCLOCK_ARM11_NEW (SYSCLOCK_ARM11 * 3)

    #define CPU_TICKS_PER_MSEC (SYSCLOCK_ARM11 / 1000.0)
    #define CPU_TICKS_PER_USEC (SYSCLOCK_ARM11 / 1000000.0)

    static Time GetCurrentTime(void)
    {
        return Microseconds(svcGetSystemTick() / CPU_TICKS_PER_USEC);
    }

    Clock::Clock(void) : _startTime(GetCurrentTime())
    {

    }

    Clock::Clock(Time time) : _startTime{ time }
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

#ifndef CTRPLUGINFRAMEWORK_UTILS_UTILS_HPP
#define CTRPLUGINFRAMEWORK_UTILS_UTILS_HPP

#include "types.h"
#include <string>

namespace CTRPluginFramework
{
    class Utils
    {
    public:

        static std::string  Format(const char *fmt, ...);
        static u32          Random(void);
        static u32          Random(u32 min, u32 max);
    };
}

#endif
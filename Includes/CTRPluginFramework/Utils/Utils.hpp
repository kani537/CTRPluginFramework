#ifndef CTRPLUGINFRAMEWORK_UTILS_UTILS_HPP
#define CTRPLUGINFRAMEWORK_UTILS_UTILS_HPP

#include <string>

namespace CTRPluginFramework
{
    class Utils
    {
    public:

        static std::string  Format(const char *fmt, ...);
    };
}

#endif
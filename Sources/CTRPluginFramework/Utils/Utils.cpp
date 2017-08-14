#include "CTRPluginFramework/Utils/Utils.hpp"
#include <cstdarg>
#include <cstdio>
#include <random>

namespace CTRPluginFramework
{
    std::string Utils::Format(const char* fmt, ...)
    {
        char        buffer[0x100] = { 0 };
        va_list     argList;

        va_start(argList, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, argList);
        va_end(argList);

        return (std::string(buffer));
    }

    static std::mt19937    g_rng; ///< Engine

    void    InitializeRandomEngine(void)
    {
        // Init the engine with a random seed
        g_rng.seed(std::random_device()());
    }

    u32     Utils::Random(void)
    {
        return (Random(0, 0));
    }

    u32     Utils::Random(u32 min, u32 max)
    {
        if (max == 0)
            max = UINT32_MAX;
        std::uniform_int_distribution<u32>  uniform(min, max);

        return (uniform(g_rng));
    }
}

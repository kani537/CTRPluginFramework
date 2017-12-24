#include "CTRPluginFramework/Utils/Utils.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <random>
#include "ctrulib/util/utf.h"
#include "ctrulib/svc.h"

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

    std::string Utils::ToHex(u32 x)
    {
        char buf[9] = { 0 };

        sprintf(buf, "%08X", x);

        return (buf);
    }

    std::string     Utils::ToString(float fpval, int precision)
    {
        return (fpval > 999999.f || fpval < -999999.f ? Format(Format("%%.%de", precision).c_str(), fpval) : Format(Format("%%.%df", precision).c_str(), fpval));
    }

    static std::mt19937    g_rng; ///< Engine

    void    InitializeRandomEngine(void)
    {
        // Init the engine with a random seed
        g_rng.seed(svcGetSystemTick());
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

    u32     Utils::GetSize(const std::string &str)
    {
        u32     size = str.length();
        u8      buffer[0x100] = { 0 };
        u8      *s = buffer;

        if (!size) return (0);

        std::memcpy(buffer, str.data(), size);

        // Skip UTF8 sig
        if (s[0] == 0xEF && s[1] == 0xBB && s[2] == 0xBF)
            s += 3;

        size = 0;
        while (*s)
        {
            u32 code;
            int units = decode_utf8(&code, s);

            if (units == -1)
                break;

            s += units;
            size++;
        }
        return (size);
    }

    u32     Utils::RemoveLastChar(std::string &str)
    {
        u32     size = str.length();
        u8      buffer[0x100] = { 0 };
        u8      *s = buffer;

        if (!size) return (0);

        std::memcpy(buffer, str.data(), size);

        // Skip UTF8 sig
        if (s[0] == 0xEF && s[1] == 0xBB && s[2] == 0xBF)
            s += 3;

        while (*s)
        {
            u32 code;
            int units = decode_utf8(&code, s);

            if (units == -1)
                break;
            if (*(s + units))
                s += units;
            else
            {
                *s = 0;
                str = reinterpret_cast<char *>(buffer);
                return (code);
            }
        }
        return (0);
    }
}

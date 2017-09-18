#include "CTRPluginFramework/Utils/StringExtensions.hpp"
#include <algorithm>

namespace CTRPluginFramework
{
    std::string     ResetColor(void)
    {
        return ("\x18");
    }

    std::string& operator<<(std::string& left, const char* right)
    {
        left += right;
        return (left);
    }

    std::string operator <<(const std::string &left, const std::string &right)
    {
        return (left + right);
    }

    std::string operator <<(const std::string &left, const Color &color)
    {
        char  strColor[5] = { 0 };

        strColor[0] = 0x1B;
        strColor[1] = std::max((u8)1, color.r);
        strColor[2] = std::max((u8)1, color.g);
        strColor[3] = std::max((u8)1, color.b);

        return (left + strColor);
    }

    std::string operator <<(const Color &color, const std::string &right)
    {
        char  strColor[5] = { 0 };

        strColor[0] = 0x1B;
        strColor[1] = std::max((u8)1, color.r);
        strColor[2] = std::max((u8)1, color.g);
        strColor[3] = std::max((u8)1, color.b);

        return (strColor + right);
    }
}

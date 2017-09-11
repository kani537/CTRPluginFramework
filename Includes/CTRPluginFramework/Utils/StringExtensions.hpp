#ifndef CTRPLUGINFRAMEWORK_UTILS_STRINGEXTENSIONS_HPP
#define CTRPLUGINFRAMEWORK_UTILS_STRINGEXTENSIONS_HPP

#include "CTRPluginFramework/Graphics/Color.hpp"
#include <string>

namespace CTRPluginFramework
{
    // Return a string with the charcter that reset the color (0x18)
    std::string     ResetColor(void);

    std::string operator <<(const std::string &left, const std::string &right);
    std::string operator <<(const std::string &left, const Color &color);
    std::string operator <<(const Color &color, const std::string &right);

    template <typename T>
    std::string &operator <<(std::string &left, T right)
    {
        left += std::to_string(right);
        return (left);
    }
}

#endif

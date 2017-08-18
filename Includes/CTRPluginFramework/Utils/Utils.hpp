#ifndef CTRPLUGINFRAMEWORK_UTILS_UTILS_HPP
#define CTRPLUGINFRAMEWORK_UTILS_UTILS_HPP

#include "types.h"
#include <string>

namespace CTRPluginFramework
{
    class Utils
    {
    public:

        /**
         * \brief Get a string formatted with format specifier from printf
         * \param fmt String to be formatted
         * \param ... Addtionnal arguments
         * \return The formatted std::string
         */
        static std::string  Format(const char *fmt, ...);

        /**
         * \brief Get a random number
         * \return A random number
         */
        static u32          Random(void);

        /**
         * \brief Get a random number
         * \param min Minimu value for the random number
         * \param max Maximum value for the random number
         * \return A random number between min & max
         */
        static u32          Random(u32 min, u32 max);

        /**
         * \brief Get the size of an utf8 std::string (max size 0x100)
         * \param str The string to count
         * \return The count of utf8 chars in the str
         */
        static u32          GetSize(const std::string &str);

        /**
         * \brief Remove the last char of an utf8 string (max size 0x100)
         * \param str The string to remove the char from
         * \return The codepoint value of the char removed
         */
        static u32          RemoveLastChar(std::string &str);
    };
}

#endif
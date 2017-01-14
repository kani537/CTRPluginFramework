#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework
{
    const Color Color::Black = Color(0, 0, 0);
    const Color Color::White = Color(255, 255, 255);
    const Color Color::Red = Color(255, 0, 0);
    const Color Color::Green = Color(0, 255, 0);
    const Color Color::Blue = Color(0, 0, 255);
    const Color Color::Yellow = Color(255, 255, 0);
    const Color Color::Magenta = Color(255, 0, 255);
    const Color Color::Cyan = Color(0, 255, 255);
    const Color Color::Transparent = Color(0, 0, 0, 0);

    Color::Color() : r(0), g(0), b(0), a(255)
    {

    }

    Color::Color(u8 red, u8 green, u8 blue, u8 alpha) 
    : r(red), g(green), b(blue), a(alpha)
    {

    }

    u32     Color::ToU32(void)
    {
        return ((a << 24) | (b << 16) | (g << 8) | r);
    }
}
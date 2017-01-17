#include "CTRPluginFramework.hpp"
#include <cmath>

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

    Color Color::ColorFromMemory(u8 *array, bool isBGR, bool haveAlpha)
    {
        u8 a, r, g, b;

        if (isBGR)
        {
            if (haveAlpha)
                a = *array++;
            else
                a = 255;
            b = *array++;
            g = *array++;
            r = *array;
        }
        else
        {
            r = *array++;
            g = *array++;
            b = *array++;
            if (haveAlpha)
                a = *array;
            else
                a = 255;
        }
        return (Color(r, g, b, a));
    }

    Color::Color(u32 color)
    {
        a = color & 0xFF;
        r = color >> 24;
        g = (color >> 16) & 0xFF;
        b = (color >> 8) & 0xFF;
    }

    u32     Color::ToU32(void)
    {
        return ((a << 24) | (b << 16) | (g << 8) | r);
    }

    Color&   Color::Fade(double fading)
    {
        if (fading > 1.0f || fading < -1.0f)
            return (*this);

        if (fading > 0.0f)
        {
            double tint = 1.f - fading;
            r = 255 - (255 - r) * tint;
            g = 255 - (255 - g) * tint;
            b = 255 - (255 - b) * tint;
        }
        else
        {
            double shade = 1.f + fading;

            r *= shade;
            g *= shade;
            b *= shade;
        }
        return (*this);
    }
}

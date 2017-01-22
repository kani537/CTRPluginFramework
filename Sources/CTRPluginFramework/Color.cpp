#include "CTRPluginFramework.hpp"
#include <cmath>
#include <algorithm>

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

    Color Color::FromMemory(u8 *src, GSPGPU_FramebufferFormats format)
    {
        u8 a, r, g, b;
        union
        {
            u16     u;
            u8      b[2];
        }     half;

        switch (format)
        {
            case GSP_RGBA8_OES:
                a = *src++;
                b = *src++;
                g = *src++;
                r = *src;
                break;
            case GSP_BGR8_OES:
                a = 255;
                b = *src++;
                g = *src++;
                r = *src;
                break;
            case GSP_RGB565_OES:
                half.b[0] = *src++;
                half.b[1] = *src;

                a = 255;
                b = (half.u >> 8) & 0xF8;
                g = (half.u >> 3) & 0xFC;
                r = (half.u << 3) & 0xF8;
                break;
            case GSP_RGB5_A1_OES:
                half.b[0] = *src++;
                half.b[1] = *src;

                a = 255;
                b = (half.u >> 8) & 0xF8;
                g = (half.u >> 3) & 0xF8;
                r = (half.u << 2) & 0xF8;
                break;
            case GSP_RGBA4_OES:
                a = 255;
                b = (half.u >> 8) & 0xF0;
                g = (half.u >> 4) & 0xF0;
                r = (half.u ) & 0xF0;
                break;            
        }
        return (Color(r, g, b, a));
    }

    void Color::ToMemory(u8 *dst, GSPGPU_FramebufferFormats format)
    {
        union
        {
            u16     u;
            u8      b[2];
        }     half;

        u8 _b;
        u8 _g;
        u8 _r;

        switch (format)
        {
            case GSP_RGBA8_OES:
                *(dst++) = a;
                _b = *(dst);
                _g = *(dst + 1);
                _r = *(dst + 2);
                
                _b *= (255 - a);
                _g *= (255 - a);
                _r *= (255 - a);

                *dst++ = ((b * a) + _b) / 255;
                *dst++ = ((g * a) + _g) / 255;
                *dst = ((r * a) + _r) / 255;


                break;
            case GSP_BGR8_OES:
                _b = *(dst);
                _g = *(dst + 1);
                _r = *(dst + 2);
                
                _b *= (255 - a);
                _g *= (255 - a);
                _r *= (255 - a);

                *dst++ = ((b * a) + _b) / 255;
                *dst++ = ((g * a) + _g) / 255;
                *dst = ((r * a) + _r) / 255;
                break;
            case GSP_RGB565_OES:
                half.u  = (b & 0xF8) << 8;
                half.u |= (g & 0xFC) << 3;
                half.u |= (r & 0xF8) >> 3;

                *(dst++) = half.b[0];
                *(dst) = half.b[1];
                break;
            case GSP_RGB5_A1_OES:
                half.u  = (b & 0xF8) << 8;
                half.u |= (g & 0xF8) << 3;
                half.u |= (r & 0xF8) >> 2;
                half.u |= 1;

                *(dst++) = half.b[0];
                *(dst) = half.b[1];
                break;
            case GSP_RGBA4_OES:
                half.u  = (b & 0xF0) << 8;
                half.u |= (g & 0xF0) << 4;
                half.u |= (r & 0xF0);
                half.u |= 0x0F;

                *(dst++) = half.b[0];
                *(dst) = half.b[1];
                break;            
        }
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
            r = std::min((int)(255 - (255 - r) * tint), 255);
            g = std::min((int)(255 - (255 - g) * tint), 255);
            b = std::min((int)(255 - (255 - b) * tint), 255);
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

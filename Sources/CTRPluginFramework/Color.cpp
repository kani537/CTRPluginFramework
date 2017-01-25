#include "CTRPluginFramework.hpp"
#include <cmath>
#include <algorithm>

namespace CTRPluginFramework
{
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
             /*   b = half.u >> 11;
                g = (half.u >> 5) & 0b111111;
                r = (half.u & 0b11111);    
                b = ( b * 527 + 23 ) >> 6;
                r = ( g * 527 + 23 ) >> 6;
                g = ( r * 259 + 33 ) >> 6;*/
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

    void Color::ToMemory(u8 *dst, GSPGPU_FramebufferFormats format, u8 *dst2)
    {
        union
        {
            u16     u;
            u8      b[2];
        }     half;

        switch (format)
        {
            case GSP_RGBA8_OES:
                *dst++  = a;
                *dst++  = b;
                *dst++  = g;
                *dst    = r;
                if (dst2)
                {
                    *dst2++ = a;
                    *dst2++ = b;
                    *dst2++ = g;
                    *dst2   = r;
                }
                break;
            case GSP_BGR8_OES:
                *dst++  = b;
                *dst++  = g;
                *dst    = r;
                if (dst2)
                {
                    *dst2++ = b;
                    *dst2++ = g;
                    *dst2   = r;
                }
                break;
            case GSP_RGB565_OES:
                half.u  = (b & 0xF8) << 8;
                half.u |= (g & 0xFC) << 3;
                half.u |= (r & 0xF8) >> 3;

                *(dst++) = half.b[0];
                *(dst) = half.b[1];
                if (dst2)
                {
                    *(dst2++) = half.b[0];
                    *(dst2) = half.b[1];                    
                }
                break;
            case GSP_RGB5_A1_OES:
                half.u  = (b & 0xF8) << 8;
                half.u |= (g & 0xF8) << 3;
                half.u |= (r & 0xF8) >> 2;
                half.u |= 1;

                *(dst++) = half.b[0];
                *(dst) = half.b[1];
                if (dst2)
                {
                    *(dst2++) = half.b[0];
                    *(dst2) = half.b[1];                    
                }
                break;
            case GSP_RGBA4_OES:
                half.u  = (b & 0xF0) << 8;
                half.u |= (g & 0xF0) << 4;
                half.u |= (r & 0xF0);
                half.u |= 0x0F;

                *(dst++) = half.b[0];
                *(dst) = half.b[1];
                if (dst2)
                {
                    *(dst2++) = half.b[0];
                    *(dst2) = half.b[1];                    
                }
                break;            
        }
    }
    void Color::ToMemoryBlend(u8 *dst, GSPGPU_FramebufferFormats format, BlendMode mode, u8 *dst2)
    {
        if (mode == BlendMode::None)
            ToMemory(dst, format, dst2);
        else if (format == GSP_RGB565_OES)
        {
            u8 _b = r;
            r = b;
            b = _b;
            ToMemory(dst, format, dst2);
        }
        else
        {
            Color original = FromMemory(dst);
            Color ret = original.Blend(*this, mode);
            ret.ToMemory(dst, format, dst2);
        }
    }

    Color::Color(u32 color)
    {
        a = color & 0xFF;
        r = color >> 24;
        g = (color >> 16) & 0xFF;
        b = (color >> 8) & 0xFF;
    }

    u32     Color::ToU32(void) const
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

    Color Color::Blend(const Color &color, BlendMode mode) const
    {
        Color           ret;
        unsigned int    _r;
        unsigned int    _g;
        unsigned int    _b;
        unsigned int    _a;

        switch (mode)
        {
        case BlendMode::Alpha:
            _r = (color.a * color.r +  (255 - color.a) * r) / 255;
            _g = (color.a * color.g + (255 - color.a) * g) / 255;
            _b = (color.a * color.b + (255 - color.a) * b) / 255;
            _a = color.a * a;
            ret.r = std::min(_r, 255u);
            ret.g = std::min(_g, 255u);
            ret.b = std::min(_b, 255u);
            ret.a = std::min(_a, 255u);
            break;
        case BlendMode::Add:
            _r = a * r / 255 + 255 * color.r / 255;
            _g = a * g / 255 + 255 * color.g / 255;
            _b = a * b / 255 + 255 * color.b / 255;
            _a = a + color.a;
            ret.r = std::min(_r, 255u);
            ret.g = std::min(_g, 255u);
            ret.b = std::min(_b, 255u);
            ret.a = std::min(_a, 255u);
            break;
        case BlendMode::Sub:
            _r = a * r / 255 - 255 * color.r / 255;
            _g = a * g / 255 - 255 * color.g / 255;
            _b = a * b / 255 - 255 * color.b / 255;
            _a = a - color.a;
            ret.r = std::max(_r, 0u);
            ret.g = std::max(_g, 0u);
            ret.b = std::max(_b, 0u);
            ret.a = std::max(_a, 0u);
            break;
        case BlendMode::Mul:
            ret = *this * color;
            break;
        default:
            ret = color;
            break;
        }
        return (ret);
    }

    bool Color::operator==(const Color& right) const
    {
        return ((r == right.r)
            && (b == right.b)
            && (g == right.g)
            && (a == right.a));
    }

    bool Color::operator!=(const Color& right) const
    {
        return ((r != right.r)
            || (b != right.b)
            || (g != right.g)
            || (a != right.a));
    }

    Color Color::operator+(const Color& right) const
    {
        u8 _r(std::min(r + right.r, 255));
        u8 _g(std::min(g + right.g, 255));
        u8 _b(std::min(b + right.b, 255));
        u8 _a(std::min(a + right.a, 255));

        return (Color(_r, _g, _b, _a));

    }

    Color Color::operator-(const Color& right) const
    {
        u8 _r(std::max(r - right.r, 0));
        u8 _g(std::max(g - right.g, 0));
        u8 _b(std::max(b - right.b, 0));
        u8 _a(std::max(a - right.a, 0));

        return (Color(_r, _g, _b, _a));
    }

    Color Color::operator*(const Color& right) const
    {
        u8 _r(r * right.r / 255);
        u8 _g(g * right.g / 255);
        u8 _b(b * right.b / 255);
        u8 _a(a * right.a / 255);

        return (Color(_r, _g, _b, _a));
    }

    Color &Color::operator+=(const Color& right)
    {
        *this = *this + right;
        return (*this);
    }

    Color &Color::operator-=(const Color& right)
    {
        *this = *this - right;
        return (*this);
    }

    Color &Color::operator*=(const Color& right)
    {
        *this = *this * right;
        return (*this);
    }
}

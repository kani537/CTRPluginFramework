#include "types.h"

#include "CTRPluginFrameworkImpl/Graphics/PrivColor.hpp"
#include "ctrulib/services/gspgpu.h"

namespace CTRPluginFramework
{
    FCPointer   PrivColor::FromFramebuffer = _ReadBGR8;
    F8Pointer   PrivColor::ToFramebuffer = _WriteBGR8;

    /*
    ** Private
    *****************/
    void    PrivColor::_SetFormat(GSPGPU_FramebufferFormats format)
    {
        switch (format)
        {
            case GSP_RGBA8_OES:
                FromFramebuffer = _ReadRGBA8;
                ToFramebuffer = _WriteRGBA8;
                break;
            case GSP_BGR8_OES:
                FromFramebuffer = _ReadBGR8;
                ToFramebuffer = _WriteBGR8;
                break;
            case GSP_RGB565_OES:
                FromFramebuffer = _ReadRGB565;
                ToFramebuffer = _WriteRGB565;
                break;
            case GSP_RGB5_A1_OES:
                FromFramebuffer = _ReadRGB5A1;
                ToFramebuffer = _WriteRGB5A1;
                break;
            case GSP_RGBA4_OES:
                FromFramebuffer = _ReadRGBA4;
                ToFramebuffer = _WriteRGBA4;
                break;            
        }
    }

    Color   PrivColor::_ReadRGBA8(u8 *src)
    {
        Color color;

        color.a = *src++;
        color.b = *src++;
        color.g = *src++;
        color.r = *src;
        return (color);
    }

    Color   PrivColor::_ReadBGR8(u8 *src)
    {
        Color color;

        color.a = 255;
        color.b = *src++;
        color.g = *src++;
        color.r = *src;
        return (color);
    }

    Color   PrivColor::_ReadRGB565(u8 *src)
    {
        union
        {
            u16     u;
            u8      b[2];
        }           half;
        Color       color;

        half.b[0] = *src++;
        half.b[1] = *src;

        color.a = 255;
        color.r = (half.u >> 8) & 0xF8;
        color.g = (half.u >> 3) & 0xFC;
        color.b = (half.u << 3) & 0xF8;
        return (color);
    }

    Color   PrivColor::_ReadRGB5A1(u8 *src)
    {
        union
        {
            u16     u;
            u8      b[2];
        }           half;
        Color       color;

        half.b[0] = *src++;
        half.b[1] = *src;

        color.a = 255;
        color.b = (half.u >> 8) & 0xF8;
        color.g = (half.u >> 3) & 0xF8;
        color.r = (half.u << 2) & 0xF8;
        return (color);
    }

    Color   PrivColor::_ReadRGBA4(u8 *src)
    {
        union
        {
            u16     u;
            u8      b[2];
        }           half;
        Color       color;

        half.b[0] = *src++;
        half.b[1] = *src;

        color.a = 255;
        color.b = (half.u >> 8) & 0xF0;
        color.g = (half.u >> 4) & 0xF0;
        color.r = half.u & 0xF0;
        return (color);
    }

    u8      *PrivColor::_WriteRGBA8(u8 *dst, Color &color)
    {
        *dst++ = color.a;
        *dst++ = color.b;
        *dst++ = color.g;
        *dst++ = color.r;
        return (dst);
    }

    u8      *PrivColor::_WriteBGR8(u8 *dst, Color &color)
    {
        *dst++ = color.b;
        *dst++ = color.g;
        *dst++ = color.r;
        return (dst);
    }

    u8      *PrivColor::_WriteRGB565(u8 *dst, Color &color)
    {
        union
        {
            u16     u;
            char    b[2];
        }           half;

        half.u  = (color.r & 0xF8) << 8;
        half.u |= (color.g & 0xFC) << 3;
        half.u |= (color.b & 0xF8) >> 3;

        *(dst++) = half.b[0];
        *(dst++) = half.b[1];
        return (dst);
    }

    u8      *PrivColor::_WriteRGB5A1(u8 *dst, Color &color)
    {
        union
        {
            u16     u;
            char    b[2];
        }           half;

        half.u  = (color.b & 0xF8) << 8;
        half.u |= (color.g & 0xF8) << 3;
        half.u |= (color.r & 0xF8) >> 2;
        half.u |= 1;

        *(dst++) = half.b[0];
        *(dst++) = half.b[1];
        return (dst);
    }

    u8      *PrivColor::_WriteRGBA4(u8 *dst, Color &color)
    {
        union
        {
            u16     u;
            char    b[2];
        }           half;

        half.u  = (color.b & 0xF0) << 8;
        half.u |= (color.g & 0xF0) << 4;
        half.u |= (color.r & 0xF0);
        half.u |= 0x0F;

        *(dst++) = half.b[0];
        *(dst++) = half.b[1];
        return (dst);
    }
}

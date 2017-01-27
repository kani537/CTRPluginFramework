#ifndef COLOR_HPP
#define COLOR_HPP

#include "types.h"
#include "ctrulib/gfx.h"

namespace CTRPluginFramework
{
    enum class BlendMode
    {
        Alpha,
        Add,
        Sub,
        Mul,
        None
    };
    class Color
    {
    public:
        Color(void) : r(0), g(0), b(0), a(0) {}
        Color(u32 color);
        Color(u8 red, u8 green, u8 blue, u8 alpha = 255);

        static Color FromMemory(u8 *src, GSPGPU_FramebufferFormats format = GSP_BGR8_OES);
        void  ToMemory(u8 *dst, GSPGPU_FramebufferFormats format = GSP_BGR8_OES, u8 *dst2 = nullptr);
        void  ToMemoryBlend(u8 *dst, GSPGPU_FramebufferFormats format = GSP_BGR8_OES, BlendMode mode = BlendMode::None, u8 *dst2 = nullptr);

        u32                 ToU32(void) const;
        Color               &Fade(double fading);   
        Color               Blend(const Color &color, BlendMode mode) const;
        

        bool operator == (const Color &right) const;
        bool operator != (const Color &right) const;
        Color operator + (const Color &right) const;
        Color operator - (const Color &right) const;
        Color operator * (const Color &right) const;
        Color &operator += (const Color &right);
        Color &operator -= (const Color &right);
        Color &operator *= (const Color &right); 

        u8      r;
        u8      g;
        u8      b;
        u8      a;
    };
}

#endif
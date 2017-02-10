#ifndef COLOR_HPP
#define COLOR_HPP

#include "ctrulib/gfx.h"
#include "CTRPluginFramework/Graphics/Color.hpp"

namespace CTRPluginFramework
{

    using FCPointer = Color (*)(u8 *);
    using F8Pointer = u8*   (*)(u8 *, Color&);

    class PrivColor
    {
    public:
        static FCPointer FromFramebuffer;
        static F8Pointer ToFramebuffer;

    private:
        friend class Screen;

        static void     _SetFormat(GSPGPU_FramebufferFormats format);

        static Color    _ReadRGBA8(u8 *src);
        static Color    _ReadBGR8(u8 *src);
        static Color    _ReadRGB565(u8 *src);
        static Color    _ReadRGB5A1(u8 *src);
        static Color    _ReadRGBA4(u8 *src);

        static u8       *_WriteRGBA8(u8 *dst, Color &color);
        static u8       *_WriteBGR8(u8 *dst, Color &color);
        static u8       *_WriteRGB565(u8 *dst, Color &color);
        static u8       *_WriteRGB5A1(u8 *dst, Color &color);
        static u8       *_WriteRGBA4(u8 *dst,  Color &color);
    };
}

#endif
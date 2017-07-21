#ifndef COLOR_HPP
#define COLOR_HPP

#include "ctrulib/gfx.h"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/System/Rect.hpp"

namespace CTRPluginFramework
{

    using FCPointer = Color (*)(u8 *);
    using F8Pointer = u8*   (*)(u8 *, Color&);

    class PrivColor
    {
    public:
        static FCPointer FromFramebuffer;
        static F8Pointer ToFramebuffer;

        static void    UseClamp(bool willUse, IntRect rect = IntRect());

        static GSPGPU_FramebufferFormats GetFormat(void);
        static void     SetFormat(GSPGPU_FramebufferFormats format);

    private:
        friend class Screen;

        

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

        static u8       *_WriteRGBA8Clamp(u8 *dst, Color &color);
        static u8       *_WriteBGR8Clamp(u8 *dst, Color &color);
        static u8       *_WriteRGB565Clamp(u8 *dst, Color &color);
        static u8       *_WriteRGB5A1Clamp(u8 *dst, Color &color);
        static u8       *_WriteRGBA4Clamp(u8 *dst,  Color &color);

        static bool     _useClamp;
        static IntRect  _clampArea;
        static GSPGPU_FramebufferFormats _format;
    };
}

#endif
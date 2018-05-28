#ifndef CTRPLUGINFRAMEWORK_SCREENIMPL_HPP
#define CTRPLUGINFRAMEWORK_SCREENIMPL_HPP

#include "ctrulib/services/gspgpu.h"
#include "CTRPluginFrameworkImpl/Graphics/BMPImage.hpp"

namespace CTRPluginFramework
{
    enum
    {
        SCREENSHOT_TOP = 1,
        SCREENSHOT_BOTTOM = 2,
        SCREENSHOT_BOTH = SCREENSHOT_TOP | SCREENSHOT_BOTTOM
    };

    class Color;
    class ScreenImpl
    {
    public:

        enum LCDSetup
        {
            WidthHeight = 0x5C,     ///< Framebuffer width & height  Lower 16 bits: width, upper 16 bits: height
            FramebufferA1 = 0x68,   ///< Framebuffer A first address For top screen, this is the left eye 3D framebuffer.
            FramebufferA2 = 0x6C,   ///< Framebuffer A second address    For top screen, this is the left eye 3D framebuffer.
            Format = 0x70,          ///< Framebuffer format  Bit0-15: framebuffer format, bit16-31: unknown
            Select = 0x78,          ///< Framebuffer select  Bit0: which framebuffer to display, bit1-7: unknown
            Stride = 0x90,          ///< Framebuffer stride  Distance in bytes between the start of two framebuffer rows (must be a multiple of 8).
            FramebufferB1 = 0x94,   ///< Framebuffer B first address For top screen, this is the right eye 3D framebuffer. Unused for bottom screen.
            FramebufferB2 = 0x98    ///< Framebuffer B second address    For top screen, this is the right eye 3D framebuffer. Unused for bottom screen.
        };

        static  ScreenImpl *Top;
        static  ScreenImpl *Bottom;

        ScreenImpl(u32 lcdSetupInfo, u32 fillColorAddress, bool isTopScreen = false);

        bool                        IsTopScreen(void);
        bool                        Is3DEnabled(void);

        void                        Flash(Color &color);

        static void                 Clean(void);
        static void                 ApplyFading(void);
        void                        Acquire(void);
        void                        Acquire(u32 left, u32 right, u32 stride, u32 format, bool backup = false);
        void                        SwapBuffer(bool flush = false, bool copy = false);

        GSPGPU_FramebufferFormats   GetFormat(void);
        u16                         GetWidth(void);
        u16                         GetHeight(void);
        u32                         GetStride(void);
        u32                         GetRowSize(void);
        u32                         GetBytesPerPixel(void);
        u32                         GetFramebufferSize(void);

        void                        GetFramebufferInfos(int &rowstride, int &bpp, GSPGPU_FramebufferFormats &format);

        u8                          *GetLeftFramebuffer(bool current = false);
        u8                          *GetLeftFramebuffer(int posX, int posY);
        u8                          *GetRightFramebuffer(bool current = false);
        u8                          *GetRightFramebuffer(int posX, int posY);
        void                        GetPosFromAddress(u32 address, int &posX, int &posY);

        void                        Fade(float fade, bool copy = false);
        void                        Flush(void);
		void						Invalidate(void);
        void                        Copy(void);
        void                        Debug(void);
        void                        ScreenToBMP(BMPImage::Pixel *bmp, u32 padding = 0);
        static BMPImage             *Screenshot(int screen, BMPImage *image = nullptr);

    private:
        friend class Renderer;
        friend void                 KeepThreadMain(void *);

        static void                 Initialize(void);

        u32                         _LCDSetup;  ///< Address of this screen LCD configuration
        u32                         _FillColor; ///< Address of this screen fill color register
        u32                        *_currentBufferReg; ///< Addres of this screen current buffer register
        u32                         _leftFramebuffers[2];
        u32                         _rightFramebuffers[2];
        u32                         _paFramebuffers[2];
        u32                        *_backupFramebuffer;
        u32                         _currentBuffer;

        u16                         _width;
        u16                         _height;
        u32                         _stride;
        u32                         _rowSize;
        u32                         _bytesPerPixel;
        bool                        _isTopScreen;
        GSPGPU_FramebufferFormats   _format;
    };
}

#endif

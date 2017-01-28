#include "types.h"
#include "ctrulib/services/gspgpu.h"


#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/Screen.hpp"
#include "CTRPluginFramework/Graphics/Renderer.hpp"

#include <cstdio>
#include <cmath>


namespace CTRPluginFramework
{
    

    Target      Renderer::_target = BOTTOM;
    bool        Renderer::_isRendering = false;
    bool        Renderer::_useDoubleBuffer = false;
    Screen      *Renderer::_screen = Screen::Bottom;
    DrawPixelP  Renderer::_DrawPixel = nullptr;
    DrawDataP   Renderer::_DrawData = nullptr;
    int         Renderer::_length = 1;
    u8          Renderer::_smallBuffer[1000] = {0};
    u8          *Renderer::_buffer = nullptr;
    u32         Renderer::_bufferSize = 1000;
    u32         Renderer::_rowstride;
    GSPGPU_FramebufferFormats Renderer::_format = GSP_BGR8_OES;


    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
    }

    void    Renderer::UseDoubleBuffer(bool useIt)
    {
        _useDoubleBuffer = useIt;
    }

    void    Renderer::InitBuffer(u32 size)
    {
        _buffer = new u8(size);
        if (_buffer == nullptr)
        {
            if (size > 0x1000)
                InitBuffer(size - 0x1000);
            else
            {
                _buffer = _smallBuffer;
                _bufferSize = 1000;
            }
        }
        else
            _bufferSize = size;
    }

    void        Renderer::Initialize(void)
    {
        _useDoubleBuffer = false;
    }

    void        Renderer::SetTarget(Target target)
    {
        _target = target;
        if (_target == BOTTOM)
            _screen = Screen::Bottom;
        else
            _screen = Screen::Top;
        _format = _screen->GetFormat();
        _rowstride = _screen->GetStride();
        switch (_format)
        {
            case GSP_RGBA8_OES:
                _DrawPixel = RenderRGBA8;
                _DrawData = RenderRGBA8;
                break;
            case GSP_BGR8_OES:
                _DrawPixel = RenderBGR8;
                _DrawData = RenderBGR8;
                break;
            case GSP_RGB565_OES:
                _DrawPixel = RenderRGB565;
                _DrawData = RenderRGB565;
                break;
            case GSP_RGB5_A1_OES:
                _DrawPixel = RenderRGB5A1;
                _DrawData = RenderRGB5A1;
                break;
            case GSP_RGBA4_OES:
                _DrawPixel = RenderRGBA4;
                _DrawData = RenderRGBA4;
                break;        
        }
    }

    void        Renderer::StartFrame(bool current)
    {
        _isRendering = true;
    }

    void        Renderer::EndFrame(bool copy)
    {

        Screen::Bottom->SwapBuffer(true, copy);
        Screen::Top->SwapBuffer(true, copy);
        gspWaitForVBlank();
        _isRendering = false;
    }

    void    Renderer::MenuSelector(int posX, int posY, int width, int height)
    {
        int x = posX;
        int y = posY;
        int w = width;
        int h = height;


        u8 *left = _screen->GetLeftFramebuffer(posX, posY + 1);
        int bpp;
        int rowstride;
        GSPGPU_FramebufferFormats fmt;
        _screen->GetFramebufferInfos(rowstride, bpp, fmt);

        // Draw Rectangle
        while (--w >= 0)
        {
            h = height;
            //x = posX + w;
            u8 *dst = left + rowstride * w;
            while (--h >= 0)
            {   
                Color c = Color::FromFramebuffer(dst);

                c.Fade(0.1f);
                Color::ToFramebuffer(dst, c);
                dst -= bpp;
            }
        }

        int tier = width / 3;
        int pitch = tier / 10;
        int j = 0;
        float fading = 0.0f;

        Color l = Color(255, 255, 255);
        posY += height;
        u8 *dst = _screen->GetLeftFramebuffer(posX + (width - tier), posY);
        u8 *rtier = dst;
        // Right tier
        for (int i = tier; i > 0; --i)
        {
            l.Fade(fading);
            Color::ToFramebuffer(rtier, l);
            j++;
            if (j == pitch)
            {
                fading -= 0.01f;
                j = 0;
            }
            rtier += rowstride;
        }

        l = Color(255, 255, 255);
        // Middle tier
        for (int i = 0; i < tier; ++i)
        {
            Color::ToFramebuffer(dst, l);
            dst -= rowstride;
        }
        fading = 0.0f;

        // Left tier
        for (int i = tier; i > 0; --i)
        {
            l.Fade(fading);
            Color::ToFramebuffer(dst, l);
            dst -= rowstride;
            j++;
            if (j == pitch)
            {
                fading -= 0.01f;
                j = 0;
            }
        }
    }
}

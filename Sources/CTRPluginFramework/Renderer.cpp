#include "CTRPluginFramework/Color.hpp"
#include "CTRPluginFramework/Renderer.hpp"
#include "CTRPluginFramework/Commands.hpp"
#include "font6x10Linux.h"
#include "3DS.h"
#include "ctrulib/services/gspgpu.h"

namespace CTRPluginFramework
{
    #define     RANGE(x, y, z) (y >= x && y <= z)

    Target      Renderer::_target = BOTTOM;
    bool        Renderer::_render3D = false;
    Screen      *Renderer::_screenTarget = nullptr;
    u8          *Renderer::_framebuffer = nullptr;
    u8          *Renderer::_framebufferP = nullptr;
    u8          *Renderer::_framebufferR = nullptr;
    u8          *Renderer::_framebufferRP = nullptr;
    u32         Renderer::_rowSize = 0;
    u32         Renderer::_targetWidth = 0;
    u32         Renderer::_targetHeight = 0;
    DrawPixelP  Renderer::_DrawPixel = nullptr;
    int         Renderer::_length = 1;

    void        Renderer::SetTarget(Target target)
    {
        _target = target;
        UpdateTarget();
    }

    void        Renderer::UpdateTarget(void)
    {
        if (_target == BOTTOM)
            _screenTarget = Screen::Bottom;
        else
            _screenTarget = Screen::Top;
        _rowSize = _screenTarget->GetRowSize();
        _targetWidth = _screenTarget->GetWidth();
        _targetHeight = _screenTarget->GetHeight();
        switch (_screenTarget->GetFormat())
        {
            case GSP_RGBA8_OES:
                _DrawPixel = RenderRGBA8;
                break;
            case GSP_BGR8_OES:
                _DrawPixel = RenderBGR8;
                break;
            case GSP_RGB565_OES:
                _DrawPixel = RenderRGB565;
                break;
            case GSP_RGB5_A1_OES:
                _DrawPixel = RenderRGB5A1;
                break;
            case GSP_RGBA4_OES:
                _DrawPixel = RenderRGBA4;
                break;
        }
    }

    void        Renderer::StartRendering(void)
    {
        _screenTarget->Update();
        _render3D = _screenTarget->Is3DEnabled();
        _framebuffer = _screenTarget->GetLeftFramebuffer();
        _framebufferP = _screenTarget->GetLeftFramebufferP();
        _framebufferR = _screenTarget->GetRightFramebuffer();
        _framebufferRP = _screenTarget->GetRightFramebufferP();

        u8  *current = _screenTarget->GetLeftFramebuffer(true);
        u32 size = _rowSize * _targetWidth * _screenTarget->GetBytesPerPixel();
        memcpy(_framebuffer, current, size);
        current = _screenTarget->GetRightFramebuffer(true);
        if (current)
            memcpy(_framebufferR, current, size);
    }

    void        Renderer::EndRendering(void)
    {
        u32 size = _rowSize * _targetWidth * _screenTarget->GetBytesPerPixel();
        ThreadCommands::SetArgs((int)_framebuffer, (int)size);
        ThreadCommands::Execute(Commands::GSPGPU_FLUSH);
       // ThreadCommands::Execute(Commands::GSPGPU_INVALIDATE);
        if (_render3D)
        {
            ThreadCommands::SetArgs((int)_framebufferR, (int)size);
            ThreadCommands::Execute(Commands::GSPGPU_FLUSH);            
            //ThreadCommands::Execute(Commands::GSPGPU_INVALIDATE);
        }
        _screenTarget->SwapBuffer();
        //ThreadCommands::Execute(Commands::GSPGPU_SWAP);
        ThreadCommands::Execute(Commands::GSPGPU_VBLANK);
        //ThreadCommands::Execute(Commands::GSPGPU_VBLANK1);
    }

    void        Renderer::DrawLine(int posX, int posY, int width, Color color, int height)
    {  
        // Correct posY
        posY += (_rowSize - 240);
        for (int x = 0; x < width; x++)
        {
            _length = height;
            _DrawPixel(posX + x, posY + height, color);
        }
    }

    void        Renderer::DrawRect(int posX, int posY, int width, int height, Color color, bool fill, int thickness)
    {
        if (fill)
        {
            DrawLine(posX, posY, width, color, height);
        }
        else
        {
            // Top line
            DrawLine(posX, posY, width, color, thickness);
            // Bottom line
            DrawLine(posX, posY + height - (thickness - 1), width, color, thickness);
            // Left line
            DrawLine(posX, posY, thickness, color, height);
            // Right line
            DrawLine(posX + width - (thickness - 1), posY, thickness, color, height);
        }
    }

    // Draw Character without background
    void     Renderer::DrawCharacter(int c, int posX, int posY, Color fg)
    {
        for (int yy = 0; yy < 10; yy++)
        {
            u8 charPos = font[c * 10 + yy];
            int x = 0;
            for (int xx = 6; xx >= 0; xx--, x++)
            {
                if ((charPos >> xx) & 1)
                {
                    _DrawPixel(posX + x, posY + yy, fg);
                }
            }
        }        
    }
    // Draw Character with background
    void     Renderer::DrawCharacter(int c, int posX, int posY, Color fg, Color bg)
    {
        for (int yy = 0; yy < 10; yy++)
        {
            u8 charPos = font[c * 10 + yy];
            int x = 0;
            for (int xx = 6; xx >= 0; xx--, x++)
            {
                if ((charPos >> xx) & 1)
                {
                    _DrawPixel(posX + x, posY + yy, fg);
                }
                else
                {
                    _DrawPixel(posX + x, posY + yy, bg);
                }
            }
        } 
    }
    // Draw Character with offset
    void     Renderer::DrawCharacter(int c, int offset, int posX, int posY, Color fg)
    {      
        for (int yy = 0; yy < 10; yy++)
        {
            u8 charPos = font[c * 10 + yy];
            int x = 0;
            for (int xx = 6 - offset; xx >= 0; xx--, x++)
            {
                if ((charPos >> xx) & 1)
                {
                    _DrawPixel(posX + x, posY + yy, fg);
                }
            }
        }  
    }

    void    Renderer::DrawString(char *str, int posX, int posY, Color fg)
    {
        // Correct posY
        posY += (_rowSize - 240);

        while (*str)
        {
            DrawCharacter(*str++, posX++, posY, fg);
            posX += 6;
        }
    }

    void    Renderer::DrawString(char *str, int posX, int posY, Color fg, Color bg)
    {
        // Correct posY
        posY += (_rowSize - 240);

        while (*str)
        {
            DrawCharacter(*str++, posX++, posY, fg, bg);
            posX += 6;
        }
    }

    void    Renderer::DrawString(char *str, int offset, int posX, int posY, Color fg)
    {
        // Correct posY
        posY += (_rowSize - 240);
        str += (offset / 6);
        offset %= 6;
        while (*str)
        {
            DrawCharacter(*str++, offset, posX, posY, fg);
            if (offset)
            {                          
                posX -= offset;      
                offset = 0;
            }
            posX += 6;
        }
    }


//############################################################################################

    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
    }

    void        Renderer::RenderRGBA8(int posX, int posY, Color color)
    {
        if (!RANGE(0, posX, _targetWidth) || !RANGE(0, posY, _targetHeight))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 4, _rowSize);
        u8      *pos = _framebuffer + offset;
        u8      *posR = _framebufferR + offset;
        while (--_length >= 0)
        {
            *(pos++) = 0xFF;
            *(pos++) = color.b;
            *(pos++) = color.g;
            *(pos++) = color.r;
            if (_render3D)
            {
                *(posR++) = 0xFF;
                *(posR++) = color.b;
                *(posR++) = color.g;
                *(posR++) = color.r;
            }         
        }
        _length = 1;
    }

    void        Renderer::RenderBGR8(int posX, int posY, Color color)
    {
        if (!RANGE(0, posX, _targetWidth) || !RANGE(0, posY, _targetHeight))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 3, _rowSize);
        u8      *pos = _framebuffer + offset;
        u8      *posR = _framebufferR + offset;

        while (--_length >= 0)
        {
            *(pos++) = color.b;
            *(pos++) = color.g;
            *(pos++) = color.r;  
            if (_render3D)
            {
                *(posR++) = color.b;
                *(posR++) = color.g;
                *(posR++) = color.r;                
            }          
        }
        _length = 1;
    }

    void        Renderer::RenderRGB565(int posX, int posY, Color color)
    {
        if (!RANGE(0, posX, _targetWidth) || !RANGE(0, posY, _targetHeight))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize);
        u8      *pos = _framebuffer + offset;
        u8      *posR = _framebufferR + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        half.u  = (color.b & 0xF8) << 8;
        half.u |= (color.g & 0xFC) << 3;
        half.u |= (color.r & 0xF8) >> 3;
        while (--_length >= 0)
        {
            *(pos++) = half.b[0];
            *(pos++) = half.b[1]; 
            if (_render3D)
            {
                *(posR++) = half.b[0];
                *(posR++) = half.b[1];              
            }              
        }
        _length = 1;
    }

    void        Renderer::RenderRGB5A1(int posX, int posY, Color color)
    {
        if (!RANGE(0, posX, _targetWidth) || !RANGE(0, posY, _targetHeight))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize);
        u8      *pos = _framebuffer + offset;
        u8      *posR = _framebufferR + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        half.u  = (color.b & 0xF8) << 8;
        half.u |= (color.g & 0xF8) << 3;
        half.u |= (color.r & 0xF8) >> 2;
        half.u |= 1;
        while (--_length >= 0)
        {
            *(pos++) = half.b[0];
            *(pos++) = half.b[1]; 
            if (_render3D)
            {
                *(posR++) = half.b[0];
                *(posR++) = half.b[1]; 
            }           
        }
        _length = 1;
    }

    void        Renderer::RenderRGBA4(int posX, int posY, Color color)
    {
        if (!RANGE(0, posX, _targetWidth) || !RANGE(0, posY, _targetHeight))
            return;

        u32     offset =  + GetFramebufferOffset(posX, posY, 2, _rowSize);
        u8      *pos = _framebuffer + offset;
        u8      *posR = _framebufferR + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        half.u  = (color.b & 0xF0) << 8;
        half.u |= (color.g & 0xF0) << 4;
        half.u |= (color.r & 0xF0);
        half.u |= 0x0F;
        while (--_length >= 0)
        {
            *(pos++) = half.b[0];
            *(pos++) = half.b[1];   
            if (_render3D)
            {
                *(posR++) = half.b[0];
                *(posR++) = half.b[1];               
            }        
        }
        _length = 1;
    }
}
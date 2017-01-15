#include "CTRPluginFramework/Color.hpp"
#include "CTRPluginFramework/Renderer.hpp"
#include "CTRPluginFramework/Commands.hpp"
#include "font6x10Linux.h"
#include "3DS.h"
#include "ctrulib/services/gspgpu.h"
#include <cstdio>


namespace CTRPluginFramework
{
    #define     RANGE(x, y, z) (y >= x && y <= z)

    Target      Renderer::_target = BOTTOM;
    bool        Renderer::_render3D = false;
    bool        Renderer::_isRendering = false;
    Screen      *Renderer::_screenTarget[2] = {Screen::Bottom, Screen::Top};
    u8          *Renderer::_framebuffer[2] = {nullptr};
    u8          *Renderer::_framebufferR[2] = {nullptr};
    u32         Renderer::_rowSize[2] = {0};
    u32         Renderer::_targetWidth[2] = {0};
    u32         Renderer::_targetHeight[2] = {0};
    DrawPixelP  Renderer::_DrawPixel = nullptr;
    DrawDataP   Renderer::_DrawData = nullptr;
    int         Renderer::_length = 1;
    u8          Renderer::_smallBuffer[1000] = {0};
    u8          *Renderer::_buffer = nullptr;
    u32         Renderer::_bufferSize = 1000;

    static FileCommand      _fileCmd;


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
        _screenTarget[BOTTOM] = Screen::Bottom;
        _screenTarget[TOP] = Screen::Top;
        InitBuffer(0x50000);
    }

    void        Renderer::SetTarget(Target target)
    {
        _target = target;

        switch (_screenTarget[_target]->GetFormat())
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

    void        Renderer::StartRenderer(bool current)
    {
        _isRendering = true;
        _screenTarget[BOTTOM]->Update();
        _framebuffer[BOTTOM] = _screenTarget[BOTTOM]->GetLeftFramebuffer(current);
        _framebufferR[BOTTOM] = 0;        
        _rowSize[BOTTOM] = _screenTarget[BOTTOM]->GetRowSize();
        _targetWidth[BOTTOM] = _screenTarget[BOTTOM]->GetWidth();
        _targetHeight[BOTTOM] = _screenTarget[BOTTOM]->GetHeight();

        // Screen TOP
        _screenTarget[TOP]->Update();
        _framebuffer[TOP] = _screenTarget[TOP]->GetLeftFramebuffer(current);
        _framebufferR[TOP] = _screenTarget[TOP]->GetRightFramebuffer(current);
        _render3D = _screenTarget[TOP]->Is3DEnabled();
        _rowSize[TOP] = _screenTarget[TOP]->GetRowSize();
        _targetWidth[TOP] = _screenTarget[TOP]->GetWidth();
        _targetHeight[TOP] = _screenTarget[TOP]->GetHeight();

        // Copy current framebuffer into the second to avoid frame glitch
        if (!current)
        {
            u8  *current = _screenTarget[BOTTOM]->GetLeftFramebuffer(true);
            int size = _screenTarget[BOTTOM]->GetFramebufferSize();
            memcpy(_framebuffer[BOTTOM], current, size);

            current = _screenTarget[TOP]->GetLeftFramebuffer(true);
            size = _screenTarget[TOP]->GetFramebufferSize();
            memcpy(_framebuffer[TOP], current, size);

            current = _screenTarget[TOP]->GetRightFramebuffer(true);
            if (current)
                memcpy(_framebufferR[TOP], current, size);
        }

    }

    void        Renderer::GetFramebuffersInfos(u32 *infos)
    {
        /*
        ** Bottom size
        ** Bottom FB
        ** Top size
        ** IS3D
        ** Top LFB
        ** Top RFB
        **/
        infos[0] = _screenTarget[BOTTOM]->GetFramebufferSize();
        infos[1] = (u32)_framebuffer[BOTTOM];

        infos[2] = _screenTarget[TOP]->GetFramebufferSize();
        infos[3] = _render3D ? 1 : 0;
        infos[4] = (u32)_framebuffer[TOP];
        infos[5] = (u32)_framebufferR[TOP];
    }

    void        Renderer::EndRenderer(void)
    {  
        ThreadCommands::Execute(Commands::GSPGPU_END);
        _isRendering = false;
    }

    void        Renderer::DrawLine(int posX, int posY, int width, Color color, int height)
    {  
        // Correct posY
        posY += (_rowSize[_target] - 240);
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

    int    Renderer::DrawString(char *str, int posX, int &posY, Color fg)
    {
        // Correct posY
        int y = posY + (_rowSize[_target] - 240);

        while (*str)
        {
            DrawCharacter(*str++, posX++, y, fg);
            posX += 6;
        }
            posY += 10;
        return (posY);
    }

    int    Renderer::DrawString(char *str, int posX, int &posY, Color fg, Color bg)
    {
        // Correct posY
        int y = posY + (_rowSize[_target] - 240);

        while (*str)
        {
            DrawCharacter(*str++, posX++, y, fg, bg);
            posX += 6;
        }
        posY += 10;
        return (posY);
    }

    int    Renderer::DrawString(char *str, int offset, int posX, int &posY, Color fg)
    {
        // Correct posY
        int y = posY + (_rowSize[_target] - 240);
        str += (offset / 6);
        offset %= 6;
        while (*str)
        {
            DrawCharacter(*str++, offset, posX, y, fg);
            if (offset)
            {                          
                posX -= offset;      
                offset = 0;
            }
            posX += 6;
        }
        posY += 10;
        return (posY);
    }
//#########################################################################################

    void    Renderer::DrawFile(std::FILE *file, int posX, int posY, int width, int height)
    {
        if (!file)
            return;


        // Init buffer
        if (_buffer == nullptr)
            return;

        int     rowsize = height * 3;
        int     fileSize = width * height * 3;
        
        // reset pos in file
        std::fseek(file, static_cast<std::size_t>(0), SEEK_SET);

        _fileCmd.file = file;        
        _fileCmd.size = fileSize;
        _fileCmd.dst = _buffer;

        _fileCmd.read = 0;
        ThreadCommands::SetArgs((int)&_fileCmd);
        ThreadCommands::Execute(Commands::FS_READFILE);
        if (_fileCmd.read == _fileCmd.size)
        {
            DrawBuffer(_buffer, posX, posY, width, height);
            return;
        }
        // Correct posY
        posY = _rowSize[_target] - posY;


        int     rowPerRead = _bufferSize / rowsize;
        int     readSize = rowPerRead * rowsize;

        int     totalRead = width / rowPerRead;
        int     leftOver = width % rowPerRead;
        totalRead += leftOver > 0 ? 1 : 0;

        _fileCmd.file = file;        
        _fileCmd.size = readSize;
        _fileCmd.dst = _buffer;

        // reset pos in file
        std::fseek(file, static_cast<std::size_t>(0), SEEK_SET);
        
        while (--totalRead >= 0)
        {
            if (totalRead > 0)
            {
                _fileCmd.read = 0;
                ThreadCommands::SetArgs((int)&_fileCmd);
                ThreadCommands::Execute(Commands::FS_READFILE);
                if (_fileCmd.read != _fileCmd.size)
                {
                    ThreadCommands::SetArgs((int)&_fileCmd);
                    ThreadCommands::Execute(Commands::FS_READFILE);
                    if (_fileCmd.read != _fileCmd.size)
                        return;
                }
                for (int i = 0; i < rowPerRead; i++)
                {
                    _DrawData(posX, posY, _buffer + (rowsize * i), height);
                    posX++;
                }
            }
            else
            {
                _fileCmd.size = leftOver * rowsize;
                _fileCmd.read = 0;
                ThreadCommands::SetArgs((int)&_fileCmd);
                ThreadCommands::Execute(Commands::FS_READFILE);
                if (_fileCmd.read != _fileCmd.size)
                {
                    ThreadCommands::SetArgs((int)&_fileCmd);
                    ThreadCommands::Execute(Commands::FS_READFILE);
                    if (_fileCmd.read != _fileCmd.size)
                        return;
                }
                for (int i = 0; i > leftOver; i++)
                {
                    _DrawData(posX, posY, _buffer + (rowsize * i), height);
                    posX++;
                }
            }            
        }
    }

    void    Renderer::DrawBuffer(u8 *buffer, int posX, int posY, int width, int height)
    {
        const int padding = height * 3;
        // Correct posY
        //posY += (_rowSize[_target] - 240);
        posY = _rowSize[_target] - posY;
        int i = 0;
        while (--width >= 0)
        {
            _DrawData(posX, posY, buffer + i, height);
            posX++;
            i += padding;
        }
    }


//############################################################################################

    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
    }

    void        Renderer::RenderRGBA8(int posX, int posY, Color color)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 4, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;
        while (--_length >= 0)
        {
            *(pos++) = 0xFF;
            *(pos++) = color.b;
            *(pos++) = color.g;
            *(pos++) = color.r;
            if (_target == TOP && _render3D)
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
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 3, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        while (--_length >= 0)
        {
            *(pos++) = color.b;
            *(pos++) = color.g;
            *(pos++) = color.r;  
            if (_target == TOP && _render3D)
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
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

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
            if (_target == TOP && _render3D)
            {
                *(posR++) = half.b[0];
                *(posR++) = half.b[1];              
            }              
        }
        _length = 1;
    }

    void        Renderer::RenderRGB5A1(int posX, int posY, Color color)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

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
            if (_target == TOP && _render3D)
            {
                *(posR++) = half.b[0];
                *(posR++) = half.b[1]; 
            }           
        }
        _length = 1;
    }

    void        Renderer::RenderRGBA4(int posX, int posY, Color color)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset =  + GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

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
            if (_target == TOP && _render3D)
            {
                *(posR++) = half.b[0];
                *(posR++) = half.b[1];               
            }        
        }
        _length = 1;
    }

 // ##################################################################################################
     void        Renderer::RenderRGBA8(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 4, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        if (!_render3D || _target == BOTTOM)
        {
            while (--height >= 0)
            {
                *(pos++) = 0xFF;
                *(pos++) = *(data++);
                *(pos++) = *(data++);
                *(pos++) = *(data++);
            }
        }
        else
        {
            while (--height >= 0)
            {
                *(pos++) = 0xFF;
                *(posR++) = 0xFF;
                *(pos++) = *(data);
                *(posR++) = *(data++);
                *(pos++) = *(data);
                *(posR++) = *(data++); 
                *(pos++) = *(data);
                *(posR++) = *(data++);   
            } 
        }       
        _length = 1;
    }

    void        Renderer::RenderBGR8(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 3, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        if (!_render3D ||_target == BOTTOM)
        {
            while (--height >= 0)
            {
                *(pos++) = *(data++);
                *(pos++) = *(data++);
                *(pos++) = *(data++);
            }
        }
        else
        {
            while (--height >= 0)
            {
                *(pos++) = *(data);
                *(posR++) = *(data++);
                *(pos++) = *(data);
                *(posR++) = *(data++); 
                *(pos++) = *(data);
                *(posR++) = *(data++); 
            }
        }
        _length = 1;
    }

    void        Renderer::RenderRGB565(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        if (!_render3D || _target == BOTTOM)
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF8) >> 3;
                half.u |= (*(data++) & 0xFC) << 3;
                half.u |= (*(data++) & 0xF8) << 8;
                
                
                *(pos++) = half.b[0];
                *(pos++) = half.b[1];                
            }
        }
        else
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF8) >> 3;
                half.u |= (*(data++) & 0xFC) << 3;
                half.u |= (*(data++) & 0xF8) << 8;;
                *(pos++) = half.b[0];                
                *(posR++) = half.b[0];
                *(pos++) = half.b[1]; 
                *(posR++) = half.b[1];                
            }
        }
        _length = 1;
    }

    void        Renderer::RenderRGB5A1(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset = GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        if (!_render3D || _target == BOTTOM)
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF8) >> 2;
                half.u |= (*(data++) & 0xF8) << 3;
                half.u |= (*(data++) & 0xF8) << 8;
                half.u |= 1;
                *(pos++) = half.b[0];
                *(pos++) = half.b[1];                
            }
        }
        else
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF8) >> 2;
                half.u |= (*(data++) & 0xF8) << 3;
                half.u |= (*(data++) & 0xF8) << 8;
                half.u |= 1;
                *(pos++) = half.b[0];                
                *(posR++) = half.b[0];
                *(pos++) = half.b[1]; 
                *(posR++) = half.b[1];                
            }
        }
        _length = 1;
    }

    void        Renderer::RenderRGBA4(int posX, int posY, u8 *data, int height)
    {
        if (!RANGE(0, posX, _targetWidth[_target]) || !RANGE(0, posY, _targetHeight[_target]))
            return;

        u32     offset =  + GetFramebufferOffset(posX, posY, 2, _rowSize[_target]);
        u8      *pos = _framebuffer[_target] + offset;
        u8      *posR = _framebufferR[_target] + offset;

        union
        {
            u16     u;
            u8      b[2];
        }     half;

        if (!_render3D || _target == BOTTOM)
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF0);
                half.u |= (*(data++) & 0xF0) << 4;
                half.u |= (*(data++) & 0xF0) << 8;
                half.u |= 0x0F;
                *(pos++) = half.b[0];
                *(pos++) = half.b[1];                
            }
        }
        else
        {
            while (--height >= 0)
            {
                half.u = (*(data++) & 0xF0);
                half.u |= (*(data++) & 0xF0) << 4;
                half.u |= (*(data++) & 0xF0) << 8;
                half.u |= 0x0F;
                *(pos++) = half.b[0];                
                *(posR++) = half.b[0];
                *(pos++) = half.b[1]; 
                *(posR++) = half.b[1];                
            }
        }
        _length = 1;
    }   
}
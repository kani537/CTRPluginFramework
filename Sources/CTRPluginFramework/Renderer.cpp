#include "CTRPluginFramework/Color.hpp"
#include "CTRPluginFramework/Renderer.hpp"
#include "3DS.h"
#include "ctrulib/services/gspgpu.h"
#include <cstdio>
#include <cmath>


namespace CTRPluginFramework
{
    

    Target      Renderer::_target = BOTTOM;
    bool        Renderer::_render3D = false;
    bool        Renderer::_isRendering = false;
    bool        Renderer::_doubleBuffer = false;
    Screen      *Renderer::_screens[2] = {Screen::Bottom, Screen::Top};
    u8          *Renderer::_framebuffer[4] = {nullptr};
    u8          *Renderer::_framebufferR[4] = {nullptr};
    u32         Renderer::_rowSize[2] = {0};
    u32         Renderer::_targetWidth[2] = {0};
    u32         Renderer::_targetHeight[2] = {0};
    DrawPixelP  Renderer::_DrawPixel = nullptr;
    DrawDataP   Renderer::_DrawData = nullptr;
    int         Renderer::_length = 1;
    u8          Renderer::_smallBuffer[1000] = {0};
    u8          *Renderer::_buffer = nullptr;
    u32         Renderer::_bufferSize = 1000;


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
        _screens[BOTTOM] = Screen::Bottom;
        _screens[TOP] = Screen::Top;
        InitBuffer(0x50000);
    }

    void        Renderer::SetTarget(Target target)
    {
        _target = target;

        switch (_screens[_target]->GetFormat())
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
        _screens[BOTTOM]->Update();
        _framebuffer[BOTTOM] = _screens[BOTTOM]->GetLeftFramebuffer(current);
        _framebufferR[BOTTOM] = 0;        
        _rowSize[BOTTOM] = _screens[BOTTOM]->GetRowSize();
        _targetWidth[BOTTOM] = _screens[BOTTOM]->GetWidth();
        _targetHeight[BOTTOM] = _screens[BOTTOM]->GetHeight();

        // Screen TOP
        _screens[TOP]->Update();
        _framebuffer[TOP] = _screens[TOP]->GetLeftFramebuffer(current);
        _framebufferR[TOP] = _screens[TOP]->GetRightFramebuffer(current);
        _render3D = _screens[TOP]->Is3DEnabled();
        _rowSize[TOP] = _screens[TOP]->GetRowSize();
        _targetWidth[TOP] = _screens[TOP]->GetWidth();
        _targetHeight[TOP] = _screens[TOP]->GetHeight();

        // Copy current framebuffer into the second to avoid frame glitch
        if (!current)
        {
            u8  *current = _screens[BOTTOM]->GetLeftFramebuffer(true);
            int size = _screens[BOTTOM]->GetFramebufferSize();
            memcpy(_framebuffer[BOTTOM], current, size);

            current = _screens[TOP]->GetLeftFramebuffer(true);
            size = _screens[TOP]->GetFramebufferSize();
            memcpy(_framebuffer[TOP], current, size);

            current = _screens[TOP]->GetRightFramebuffer(true);
            if (current)
                memcpy(_framebufferR[TOP], current, size);
        }

    }

    void        Renderer::EndFrame(void)
    {
        Screen::Top->SwapBuffer();
        Screen::Bottom->SwapBuffer();
        gspWaitForVBlank();
        gspWaitForVBlank1();
        _isRendering = false;
    }

    inline u32   GetFramebufferOffset(int posX, int posY, int bpp, int rowsize)
    {
        return ((rowsize - 1 - posY + posX * rowsize) * bpp);
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

    void    Renderer::DrawFile(std::FILE *file, int posX, int posY, int width, int height)
    {
        
            return;

/*
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
        }*/
    }

    void    Renderer::DrawBuffer(u8 *buffer, int posX, int posY, int width, int height)
    {
        const int padding = height * 3;

        // Correct posY
        posY = _rowSize[_target] - posY;

        int i = 0;
        while (--width >= 0)
        {
            _DrawData(posX, posY, buffer + i, height);
            posX++;
            i += padding;
        }

    }
}

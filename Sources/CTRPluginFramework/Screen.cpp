#include "types.h"
#include "3DS.h"

#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/Graphics/Renderer.hpp"
#include "CTRPluginFramework/Screen.hpp"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFramework/Process.hpp"
#include "CTRPluginFramework/Time.hpp"
#include "CTRPluginFramework/Sleep.hpp"

#include <cstdio>


namespace CTRPluginFramework
{

    #define REG(x) *(vu32 *)(x)

    Screen *Screen::Top = nullptr;
    Screen  *Screen::Bottom = nullptr;

    extern "C" void    UpdateCtrulibGfx(void);

    void    UpdateCtrulibGfx(void)
    {
        Screen::Top->SetCtrulibScreen();
        Screen::Bottom->SetCtrulibScreen();
    }

    void    Screen::SetCtrulibScreen(void)
    {
        if (_isTopScreen)
        {
            gfxTopLeftFramebuffers[0] = (u8 *)_leftFramebuffersV[0];
            gfxTopLeftFramebuffers[1] = (u8 *)_leftFramebuffersV[1];
            gfxTopRightFramebuffers[0] = (u8 *)_rightFramebuffersV[0];
            gfxTopRightFramebuffers[1] = (u8 *)_rightFramebuffersV[1];           
        }
        else
        {
            gfxBottomFramebuffers[0] = (u8 *)_leftFramebuffersV[0];
            gfxBottomFramebuffers[1] = (u8 *)_leftFramebuffersV[1];
        }
    }

    u32     FromPhysicalToVirtual(u32 address)
    {
        if (address >= 0x18000000 && address <= 0x20000000)
            address += 0x07000000;
        else if (address >= 0x20000000 && address <= 0x30000000)
        {
            u32     values[2] = {0};

            svcGetProcessInfo((s64 *)&values, Process::GetHandle(), 20);
            if (values[0] == 0xF0000000)
                address += 0x10000000;
            else
                address -= values[0];
        }
        else
            return (0);
        return (address);
    }

    u32 GetBPP(GSPGPU_FramebufferFormats format)
    {
        switch(format) 
        {
            case GSP_RGBA8_OES:
                return 4;
            case GSP_BGR8_OES:
                return 3;
            case GSP_RGB565_OES:
            case GSP_RGB5_A1_OES:
            case GSP_RGBA4_OES:
                return 2;
        }
        return 3;
    }
// ##########################################################################

    Screen::Screen(u32 lcdSetupInfo, u32 fillColorAddress, bool isTopScreen) : 
    _LCDSetup(lcdSetupInfo),
    _FillColor(fillColorAddress),
    _isTopScreen(isTopScreen)
    {
        // Get format
        _format = (GSPGPU_FramebufferFormats)(REG(_LCDSetup + LCDSetup::Format) & 0b111);

        // Get width & height
        u32 wh = REG(_LCDSetup + LCDSetup::WidthHeight);
        _height = (u16)(wh & 0xFFFF);
        _width = (u16)(wh >> 16);

        // Get left framebuffers pointers (Physical need to be converted)
        _leftFramebuffersP[0] = REG(_LCDSetup + LCDSetup::FramebufferA1);
        _leftFramebuffersP[1] = REG(_LCDSetup + LCDSetup::FramebufferA2);

        // Converting the framebuffers
        _leftFramebuffersV[0] = FromPhysicalToVirtual(_leftFramebuffersP[0]);
        _leftFramebuffersV[1] = FromPhysicalToVirtual(_leftFramebuffersP[1]);

        if (isTopScreen)
        {
            // Get right framebuffers pointers (Physical need to be converted)
            _rightFramebuffersP[0] = REG(_LCDSetup + LCDSetup::FramebufferB1);
            _rightFramebuffersP[1] = REG(_LCDSetup + LCDSetup::FramebufferB2);  

            // Converting the framebuffers
            _rightFramebuffersV[0] = FromPhysicalToVirtual(_rightFramebuffersP[0]);
            _rightFramebuffersV[1] = FromPhysicalToVirtual(_rightFramebuffersP[1]);
        }
        else
        {
            _rightFramebuffersP[0] = 0;
            _rightFramebuffersP[1] = 0;
            _rightFramebuffersV[0] = 0;
            _rightFramebuffersV[1] = 0;
        }


        // Set current buffer pointers
        _currentBufferReg = (u32 *)(_LCDSetup + LCDSetup::Select);
        _currentBuffer = 0;

        // Get stride
        _stride = REG(_LCDSetup + LCDSetup::Stride);

        // Set bytes per pixel
        _bytesPerPixel = GetBPP(GetFormat());

        // Set row size
        _rowSize = _stride / _bytesPerPixel;

    }

    void    Screen::Initialize(void)
    {
        Screen::Top = new Screen(System::GetIOBasePDC() + 0x400, System::GetIOBaseLCD() + 0x204, true);
        Screen::Bottom = new Screen(System::GetIOBasePDC() + 0x500, System::GetIOBaseLCD() + 0xA04);
    }

    void    Screen::Fade(float fade, bool copy)
    {
        *_currentBufferReg = 0;       

        u32 size = GetFramebufferSize();

        u8 *src = GetLeftFramebuffer();
        u8 *src1 = GetRightFramebuffer();

        u8 *src3 = copy ? GetLeftFramebuffer(true) : nullptr;
        u8 *src4 = copy ? GetRightFramebuffer(true) : nullptr;

        for (int i = 0; i < size; i += _bytesPerPixel)
        {
            Color c = Color::FromMemory(src, _format);

            c.Fade(fade);
            c.ToMemory(src, _format, src3);
            if (Is3DEnabled())
            {
                Color d = Color::FromMemory(src1, _format);

                d.Fade(fade);
                d.ToMemory(src1, _format, src4);
                src1 += _bytesPerPixel;
            }
            src += _bytesPerPixel;
            if (copy)
            {
                src3 += _bytesPerPixel;
                src4 += _bytesPerPixel;
            }       
        }
    }

    void    Screen::Acquire(void)
    {
        
    again:
        u32     leftFB1 = REG(_LCDSetup + FramebufferA1);
        u32     leftFB2 = REG(_LCDSetup + FramebufferA2);
        u32     rightFB1 = REG(_LCDSetup + FramebufferB1);
        u32     rightFB2 = REG(_LCDSetup + FramebufferB2);

        if (leftFB1 == leftFB2)
        {
            u16 sl = svcGetSystemTick() & 0xFF;
            Sleep(Microseconds(sl));
            goto again;
        }

        _currentBuffer = *_currentBufferReg;
        // Get format
        _format = (GSPGPU_FramebufferFormats)(REG(_LCDSetup + LCDSetup::Format) & 0b111);

        // Get width & height
        u32 wh = REG(_LCDSetup + LCDSetup::WidthHeight);
        _height = (u16)(wh & 0xFFFF);
        _width = (u16)(wh >> 16);

        // Get stride
        _stride = REG(_LCDSetup + LCDSetup::Stride);

        // Set bytes per pixel
        _bytesPerPixel = GetBPP(GetFormat());

        // Set row size
        _rowSize = _stride / _bytesPerPixel;

        _leftFramebuffersP[0] = leftFB1;
        _leftFramebuffersP[1] = leftFB2;
        _leftFramebuffersV[0] = FromPhysicalToVirtual(leftFB1);
        _leftFramebuffersV[1] = FromPhysicalToVirtual(leftFB2);

        if (_isTopScreen)
        {
            _rightFramebuffersP[0] = rightFB1;
            _rightFramebuffersP[1] = rightFB2;
            _rightFramebuffersV[0] = FromPhysicalToVirtual(rightFB1);
            _rightFramebuffersV[1] = FromPhysicalToVirtual(rightFB2);
        }

        u32 size = GetFramebufferSize();

        // Flush left framebuf0
        if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[0], size)))
            svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[0], size);
        // Copy left framebuf0 to framebuf1
        memcpy((void *)_leftFramebuffersV[1], (void *)_leftFramebuffersV[0], size);
        // Flush left framebuf1
        if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[1], size)))
            svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[1], size);

        if (Is3DEnabled())
        {
            // Flush right framebuf0
            if (R_FAILED(GSPGPU_FlushDataCache((void *)_rightFramebuffersV[0], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffersV[0], size);
            // Copy right framebuf0 to framebuf1      
            memcpy((void *)_rightFramebuffersV[1], (void *)_rightFramebuffersV[0], size);
            //
            // Flush right framebuf1
            if (R_FAILED(GSPGPU_FlushDataCache((void *)_rightFramebuffersV[1], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffersV[1], size);
        }
    }

    bool    Screen::IsTopScreen(void)
    {
        return (_isTopScreen);
    }

    bool    Screen::Is3DEnabled(void)
    {
        if (!_isTopScreen)
            return (false);

        return (*(float *)(0x1FF81080) > 0.f ? true : false);
    }

    void    Screen::Flash(Color &color)
    {
        u32     fillColor = (color.ToU32() & 0xFFFFFF) | 0x01000000;

        for (int i = 0; i < 0x64; i++)
        {
            REG(_FillColor) = fillColor;
            svcSleepThread(5000000); // 0.005 second
        }
        REG(_FillColor) = 0;
    }

    void    Screen::SwapBuffer(bool flush, bool copy)
    {
        if (flush)
        {
            u32 size = GetFramebufferSize();

            if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[!_currentBuffer], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[!_currentBuffer], size);
            if (Is3DEnabled())
                if (R_FAILED(GSPGPU_FlushDataCache((void *)_rightFramebuffersV[!_currentBuffer], size)))
                    svcFlushProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffersV[!_currentBuffer], size);
        }
        if (copy)
        {
            u32 size = GetFramebufferSize();
            memcpy((void *)_leftFramebuffersV[_currentBuffer], (void *)_leftFramebuffersV[!_currentBuffer], size);
            if (Is3DEnabled())
                memcpy((void *)_rightFramebuffersV[_currentBuffer], (void *)_rightFramebuffersV[!_currentBuffer], size);
        }

        // Change buffer
        _currentBuffer = !_currentBuffer;

        // Set gpu buffer
        *_currentBufferReg = _currentBuffer;
        // Ensure that the framebuffers are the good ones
        REG(_LCDSetup + FramebufferA1) = _leftFramebuffersP[0];
        REG(_LCDSetup + FramebufferA2) = _leftFramebuffersP[1];
        REG(_LCDSetup + FramebufferB1) = _rightFramebuffersP[0];
        REG(_LCDSetup + FramebufferB2) = _rightFramebuffersP[1];
    }

    GSPGPU_FramebufferFormats   Screen::GetFormat(void)
    {
        return (_format);
    }

    u16     Screen::GetWidth(void)
    {
        return (_width);
    }

    u16     Screen::GetHeight(void)
    {
        return (_height);
    }

    u32     Screen::GetStride(void)
    {
        return (_stride);
    }

    u32     Screen::GetRowSize(void)
    {
        return (_rowSize);
    }

    u32     Screen::GetBytesPerPixel(void)
    {
        return (_bytesPerPixel);
    }

    u32     Screen::GetFramebufferSize(void)
    {
        return (_stride * _width);
    }

    void    Screen::GetFramebufferInfos(int &rowstride, int &bpp, GSPGPU_FramebufferFormats &format)
    {
        rowstride = _stride;
        bpp = _bytesPerPixel;
        format = _format;
    }

    int    Screen::Debug(int posX, int posY)
    {
        char buffer[50];
        Color blank = Color(255, 255, 255);

        sprintf(buffer, "left0: %08X", _leftFramebuffersV[0]);
        Renderer::DrawString(buffer, posX, posY, blank);
        //posY += 10;

        sprintf(buffer, "left1: %08X", _leftFramebuffersV[1]);
        Renderer::DrawString(buffer, posX, posY, blank);

        sprintf(buffer, "GPU left0: %08X", REG(_LCDSetup + FramebufferA1));
        Renderer::DrawString(buffer, posX, posY, blank);
        //posY += 10;

        sprintf(buffer, "GPU left1: %08X", REG(_LCDSetup + FramebufferA2));
        Renderer::DrawString(buffer, posX, posY, blank);
        //posY += 10;

        sprintf(buffer, "right0: %08X", _rightFramebuffersV[0]);
        Renderer::DrawString(buffer, posX, posY, blank);
        //posY += 10;

        sprintf(buffer, "right1: %08X", _rightFramebuffersV[1]);
        Renderer::DrawString(buffer, posX, posY, blank);
       // posY += 10;

        sprintf(buffer, "format: %d", _format);
        Renderer::DrawString(buffer, posX, posY, blank);
        //posY += 10;

        sprintf(buffer, "stride: %d, w: %d, h: %d", _stride, _width, _height);
        Renderer::DrawString(buffer, posX, posY, blank);
       // posY += 10;
        /*u32 io = System::GetIOBasePDC();
        sprintf(buffer, "GPU Busy: %08X, P3D: %08X", REG(io + 0x34), 0);
        Renderer::DrawString(buffer, posX, posY, blank);*/
        //posY += 10;

        return (posY);
    }

    /*
    ** Framebuffers Getters
    ******************************/

    /*
    ** Left
    *************/
    u8      *Screen::GetLeftFramebuffer(bool current)
    {
        return ((u8 *)_leftFramebuffersV[ current ? _currentBuffer : !_currentBuffer ]);            
    }

    u8      *Screen::GetLeftFramebuffer(int posX, int posY)
    {
        posX = std::max(posX, 0);
        posX = std::min(posX, (_isTopScreen ? 400 : 320));
        posY = std::max(posY, 0);
        posY = std::min(posY, 240);

        // Correct posY
        posY += _rowSize - 240;

        u32 offset = (_rowSize - 1 - posY + posX * _rowSize) * _bytesPerPixel;

        return ((u8 *)_leftFramebuffersV[!_currentBuffer] + offset);            
    }

    /*
    ** Right
    *************/

    u8      *Screen::GetRightFramebuffer(bool current)
    {
        if (!_isTopScreen)
            return (nullptr);

        return ((u8 *)_rightFramebuffersV[ current ? _currentBuffer : !_currentBuffer ]);            
    }

    u8      *Screen::GetRightFramebuffer(int posX, int posY)
    {
        if (!_isTopScreen)
            return (nullptr);

        posX = std::max(posX, 0);
        posX = std::min(posX, (_isTopScreen ? 400 : 320));
        posY = std::max(posY, 0);
        posY = std::min(posY, 240);
        
        // Correct posY
        posY += _rowSize - 240;
        u32 offset = (_rowSize - 1 - posY + posX * _rowSize) * _bytesPerPixel;

        return ((u8 *)_rightFramebuffersV[!_currentBuffer] + offset);            
    }
}
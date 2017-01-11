#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include <new>

namespace CTRPluginFramework
{
    //static  char    topScreenMemory[sizeof(Screen)];
    //static  char    bottomScreenMemory[sizeof(Screen)];

    #define REG(x) *(vu32 *)(x)

    Screen *Screen::Top = NULL;
    Screen  *Screen::Bottom = NULL;

    u32     FromPhysicalToVirtual(u32 address)
    {
        if (address >= 0x18000000 && address <= 0x20000000)
            address += 0x07000000;
        else if (address >= 0x20000000 && address <= 0x30000000)
        {
            /*s64     value = 0;
            svcGetProcessInfo(&value, 0xFFFF8001, 20);
            s32     offset =  (s32)(value & 0xFFFFFFFF);
            if (offset == 0)
                return (value);
            //else if (offset < 0x10000000)
                address -= offset;
            /*else
                address -= offset;*/
            address += 0x10000000;
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

    Screen::Screen(u32 lcdSetupInfo, u32 fillColorAddress) : 
    _LCDSetup(lcdSetupInfo),
    _FillColor(fillColorAddress),
    _isInit(true)
    {
        // Get format
        _format = (GSPGPU_FramebufferFormats)(REG(_LCDSetup + LCDSetup::Format) & 0xFFFF);

        // Get width & height
        u32 wh = REG(_LCDSetup + LCDSetup::WidthHeight);
        _width = (u16)(wh & 0xFFFF);
        _height = (u16)(wh >> 16);

        // Get left framebuffers pointers (Physical need to be converted)
        _leftFramebuffers[0] = REG(_LCDSetup + LCDSetup::FramebufferA1);
        _leftFramebuffers[1] = REG(_LCDSetup + LCDSetup::FramebufferA2);

        // Get right framebuffers pointers (Physical need to be converted)
        _rightFramebuffers[0] = REG(_LCDSetup + LCDSetup::FramebufferB1);
        _rightFramebuffers[1] = REG(_LCDSetup + LCDSetup::FramebufferB2);

        // Set current buffer pointers
        _currentBuffer = _LCDSetup + LCDSetup::Select;

        // Get stride
        _stride = REG(_LCDSetup + LCDSetup::Stride);

        // Set bytes per pixel
        _bytesPerPixel = GetBPP(_format);

        // Set row size
        _rowSize = _stride / _bytesPerPixel;

        // Converting the framebuffers
        _leftFramebuffers[0] = FromPhysicalToVirtual(_leftFramebuffers[0]);
        _leftFramebuffers[1] = FromPhysicalToVirtual(_leftFramebuffers[1]);
        _rightFramebuffers[0] = FromPhysicalToVirtual(_rightFramebuffers[0]);
        _rightFramebuffers[1] = FromPhysicalToVirtual(_rightFramebuffers[1]);

    }

    void    Screen::Initialize(void)
    {
        Screen::Top = new Screen(System::GetIOBasePDC() + 0x400, System::GetIOBaseLCD() + 0x204);
        Screen::Bottom = new Screen(System::GetIOBasePDC() + 0x500, System::GetIOBaseLCD() + 0xA04);
    }

    void    Screen::Flash(Color &color)
    {
        if (!_isInit)
            return;
        u32     fillColor = (color.ToU32() & 0xFFFFFF) | 0x01000000;

        for (int i = 0; i < 0x64; i++)
        {
            REG(_FillColor) = fillColor;
            svcSleepThread(5000000); // 0.005 second
        }
        REG(_FillColor) = 0;
    }

    void    Screen::SwapBuffer(void)
    {
        REG(_currentBuffer) = !REG(_currentBuffer);
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

    u8      *Screen::GetLeftFramebuffer(bool current)
    {
        u32    index = REG(_currentBuffer) & 0b1;

        if (current)
        {
            return ((u8 *)_leftFramebuffers[index]); 
        }
        return ((u8 *)_leftFramebuffers[!index]);            
    }

    u8      *Screen::GetRightFramebuffer(bool current)
    {
        u32    index = REG(_currentBuffer) & 0b1;

        if (current)
        {
            return ((u8 *)_rightFramebuffers[index]); 
        }
        return ((u8 *)_rightFramebuffers[!index]);            
    }
}
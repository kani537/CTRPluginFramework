#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include <new>

namespace CTRPluginFramework
{
    static  char    topScreenMemory[sizeof(Screen)];
    static  char    bottomScreenMemory[sizeof(Screen)];

    Screen *Screen::Top = NULL;
    Screen  *Screen::Bottom = NULL;

    u32     FromPhysicalToVirtual(u32 address)
    {
        if (address >= 0x18000000 && address <= 0x20000000)
            address += 0x07000000;
        else if (address >= 0x20000000 && address <= 0x30000000)
        {
            s64     value = 0;
            svcGetProcessInfo(&value, 0xFFFF8001, 20);
            s32     offset = (s32)(value & 0xFFFFFFFF);
            if (value == 0)
                return (value);
            else if (offset < 0x10000000)
                address -= offset;
            else
                address += offset;
        }
        else
            return (0);
        return (address);
    }

    Screen::Screen(u32 lcdSetupInfo, u32 fillColorAddress) : 
    _LCDSetup((u32 *)(lcdSetupInfo)),
    _FillColor((u32 *)fillColorAddress),
    _isInit(true)
    {
        // Get format
        _format = (GSPGPU_FramebufferFormats)(*(u32 *)(_LCDSetup + LCDSetup::Format) & 0xFFFF);

        // Get width & height
        u32 wh = *(u32 *)(_LCDSetup + LCDSetup::WidthHeight);
        _width = (u16)(wh & 0xFFFF);
        _height = (u16)(wh >> 16);

        // Get left framebuffers pointers (Physical need to be converted)
        _leftFramebuffers[0] = (u8 *)(*(u32 *)(_LCDSetup + LCDSetup::FramebufferA1));
        _leftFramebuffers[1] = (u8 *)(*(u32 *)(_LCDSetup + LCDSetup::FramebufferA2));

        // Get right framebuffers pointers (Physical need to be converted)
        _rightFramebuffers[0] = (u8 *)(*(u32 *)(_LCDSetup + LCDSetup::FramebufferB1));
        _rightFramebuffers[1] = (u8 *)(*(u32 *)(_LCDSetup + LCDSetup::FramebufferB2));

        // Set current buffer pointers
        _currentBuffer = (u32 *)(_LCDSetup + LCDSetup::Select);

        // Get stride
        _stride = *(u32 *)(_LCDSetup + LCDSetup::Stride);

        // Converting the framebuffers
        _leftFramebuffers[0] = (u8 *)FromPhysicalToVirtual((u32)_leftFramebuffers[0]);
        _leftFramebuffers[1] = (u8 *)FromPhysicalToVirtual((u32)_leftFramebuffers[1]);
        _rightFramebuffers[0] = (u8 *)FromPhysicalToVirtual((u32)_rightFramebuffers[0]);
        _rightFramebuffers[1] = (u8 *)FromPhysicalToVirtual((u32)_rightFramebuffers[1]);

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
            *(_FillColor) = fillColor;
            svcSleepThread(5000000); // 0.005 second
        }
        *(_FillColor) = 0;
    }

    void    Screen::SwapBuffer(void)
    {
        *_currentBuffer = !*_currentBuffer;
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

    u8      *Screen::GetLeftFramebuffer(bool current)
    {
        u32    index = *_currentBuffer & 0b1;

        if (current)
        {
            return (_leftFramebuffers[index]); 
        }
        return (_leftFramebuffers[!index]);            
    }

    u8      *Screen::GetRightFramebuffer(bool current)
    {
        u32    index = *_currentBuffer & 0b1;

        if (current)
        {
            return (_rightFramebuffers[index]); 
        }
        return (_rightFramebuffers[!index]);            
    }
}
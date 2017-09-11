#include "types.h"
#include "3DS.h"

#include "CTRPluginFramework/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include <cstring>
#include <cstdio>
#include "NTR.hpp"


namespace CTRPluginFramework
{

    #define REG(x) *(vu32 *)(x)

    u8  _topBuf[sizeof(Screen)];
    u8  _botBuf[sizeof(Screen)];

    Screen  *Screen::Top = nullptr;
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
        Screen::Top = new (_topBuf) Screen(SystemImpl::GetIOBasePDC() + 0x400, SystemImpl::GetIOBaseLCD() + 0x204, true);
        Screen::Bottom = new (_botBuf) Screen(SystemImpl::GetIOBasePDC() + 0x500, SystemImpl::GetIOBaseLCD() + 0xA04);
    }


    void    Screen::Fade(float fade, bool copy)
    {
        u32 size = GetFramebufferSize() / _bytesPerPixel;

        u8 *framebuf = (u8 *)_leftFramebuffersV[!_currentBuffer];

        for (int i = size; i > 0; i--)
        {
            Color c = PrivColor::FromFramebuffer(framebuf);
            
            c.Fade(fade);
            framebuf = PrivColor::ToFramebuffer(framebuf, c);
        }
    }

    void    Screen::Acquire(OSDParams &params)
    {
        memset(&params, 0, sizeof(params));
        params.isBottom = !_isTopScreen;
        params.is3DEnabled = Is3DEnabled();
        params.leftFramebuffer = *_currentBufferReg & 1u ? REG(_LCDSetup + FramebufferA2) : REG(_LCDSetup + FramebufferA1);
        params.leftFramebuffer = FromPhysicalToVirtual(params.leftFramebuffer);

        if (_isTopScreen)
        {
            params.rightFramebuffer = *_currentBufferReg & 1u ? REG(_LCDSetup + FramebufferB2) : REG(_LCDSetup + FramebufferB1);
            params.rightFramebuffer = FromPhysicalToVirtual(params.rightFramebuffer);
        }

        params.stride = REG(_LCDSetup + LCDSetup::Stride);
        params.format = REG(_LCDSetup + LCDSetup::Format) & 0b111;
        params.screenWidth = REG(_LCDSetup + LCDSetup::WidthHeight) >> 16;

        u32 size = params.stride * params.screenWidth;

        // Invalidate buffer
       GSPGPU_InvalidateDataCache((void *)params.leftFramebuffer, size);

        if (Is3DEnabled())
            GSPGPU_InvalidateDataCache((void *)params.rightFramebuffer, size);
    }

    void    Screen::Acquire(bool acquiringOSD)
    {
        
    again:
        u32     leftFB1 = REG(_LCDSetup + FramebufferA1);
        u32     leftFB2 = REG(_LCDSetup + FramebufferA2);

        if (leftFB1 == leftFB2 && !acquiringOSD)
        {
            u16 sl = svcGetSystemTick() & 0xFF;
            Sleep(Microseconds(sl));
            goto again;
        }

        _currentBuffer = *_currentBufferReg & 1u;
        // Get format
        _format = (GSPGPU_FramebufferFormats)(REG(_LCDSetup + LCDSetup::Format) & 0b111);
        PrivColor::SetFormat(_format);
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

        if (_isTopScreen && !acquiringOSD)
        {
            _rightFramebuffersP[0] = leftFB1;
            _rightFramebuffersP[1] = leftFB2;
            _rightFramebuffersV[0] = FromPhysicalToVirtual(leftFB1);
            _rightFramebuffersV[1] = FromPhysicalToVirtual(leftFB2);
        }
        else if (_isTopScreen)
        {
            _rightFramebuffersP[0] = REG(_LCDSetup + FramebufferB1);;
            _rightFramebuffersP[1] = REG(_LCDSetup + FramebufferB2);;
            _rightFramebuffersV[0] = FromPhysicalToVirtual(_rightFramebuffersP[0]);
            _rightFramebuffersV[1] = FromPhysicalToVirtual(_rightFramebuffersP[1]);            
        }

        if (!acquiringOSD)
        {
            u32 size = GetFramebufferSize();
            // Flush currentBuffer
            if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[_currentBuffer], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[_currentBuffer], size);

            // Invalidate other buffer
            svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[!_currentBuffer], size);

            // Copy current buffer in the other one
            std::memcpy((void *)_leftFramebuffersV[!_currentBuffer], (void *)_leftFramebuffersV[_currentBuffer], size);
            // Flush second buffer
            if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[!_currentBuffer], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[!_currentBuffer], size);  
        }
        else
        {
            Flush();
        }      
    }

    void    Screen::Flush(void)
    {
        u32 size = GetFramebufferSize();
        // Flush currentBuffer
        if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[_currentBuffer], size)))
            svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[_currentBuffer], size);

        // Flush second buffer
        if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[!_currentBuffer], size)))
            svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[!_currentBuffer], size);

        if (Is3DEnabled())
        {
            if (R_FAILED(GSPGPU_FlushDataCache((void *)_rightFramebuffersV[_currentBuffer], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffersV[_currentBuffer], size);

            // Flush second buffer
            if (R_FAILED(GSPGPU_FlushDataCache((void *)_rightFramebuffersV[!_currentBuffer], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffersV[!_currentBuffer], size); 
                       
        }
    }

	void	Screen::Invalidate(void)
	{
		u32 size = GetFramebufferSize();

		// Invalidate currentBuffer
		if (R_FAILED(GSPGPU_InvalidateDataCache((void *)_leftFramebuffersV[_currentBuffer], size)))
			svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[_currentBuffer], size);

    	// Invalidate second buffer
		if (R_FAILED(GSPGPU_InvalidateDataCache((void *)_leftFramebuffersV[!_currentBuffer], size)))
			svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[!_currentBuffer], size);

		if (Is3DEnabled())
		{
			if (R_FAILED(GSPGPU_InvalidateDataCache((void *)_rightFramebuffersV[_currentBuffer], size)))
				svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffersV[_currentBuffer], size);

			// Invalidate second buffer
			if (R_FAILED(GSPGPU_InvalidateDataCache((void *)_rightFramebuffersV[!_currentBuffer], size)))
				svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffersV[!_currentBuffer], size);
		}
	}

	void    Screen::Copy(void)
    {
        u32 size = GetFramebufferSize();

        // Flush currentBuffer
        if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[_currentBuffer], size)))
            svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[_currentBuffer], size);


        // Copy current buffer in the other one
        memcpy((void *)_leftFramebuffersV[!_currentBuffer], (void *)_leftFramebuffersV[_currentBuffer], size);
        // Flush second buffer
        if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[!_currentBuffer], size)))
            svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[!_currentBuffer], size);         
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

    /*
    ** Swap buffers
    *****************/

    void    Screen::SwapBuffer(bool flush, bool copy)
    {   
        if (flush)
        {
            u32 size = GetFramebufferSize();

            if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[!_currentBuffer], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[!_currentBuffer], size);
        }
        
        // Change buffer
        _currentBuffer = !_currentBuffer;

        // Set gpu buffer
        if (!_currentBuffer)
            *_currentBufferReg &= ~1u;
        else
            *_currentBufferReg |= 1u;

        if (copy)
        {
            u32 size = GetFramebufferSize();
            memcpy((void *)_leftFramebuffersV[!_currentBuffer], (void *)_leftFramebuffersV[_currentBuffer], size);

            if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffersV[!_currentBuffer], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffersV[!_currentBuffer], size);
        }


        // Ensure that the framebuffers are the good ones
        u32 left1 = _leftFramebuffersP[0];
        REG(_LCDSetup + FramebufferA1) = left1;
        REG(_LCDSetup + FramebufferB1) = left1; 

        u32 left2 = _leftFramebuffersP[1];
        REG(_LCDSetup + FramebufferA2) = left2;
        REG(_LCDSetup + FramebufferB2) = left2;
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

    u8      *Screen::GetLeftFramebuffer(int posX, int posY, bool second)
    {
        posX = std::max(posX, 0);
        posX = std::min(posX, (_isTopScreen ? 400 : 320));
        posY = std::max(posY, 0);
        posY = std::min(posY, 240);

        // Correct posY
        posY += _rowSize - 240;

        u32 offset = (_rowSize - 1 - posY + posX * _rowSize) * _bytesPerPixel;

        return ((u8 *)_leftFramebuffersV[_currentBuffer] + offset);            
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

    u8      *Screen::GetRightFramebuffer(int posX, int posY, bool current)
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

        return ((u8 *)_rightFramebuffersV[_currentBuffer] + offset);            
    }

    void    Screen::GetPosFromAddress(u32 addr, int &posX, int &posY)
    {
        addr -= _leftFramebuffersV[!_currentBuffer];

        posX = addr / (_rowSize * _bytesPerPixel);
        posY = _rowSize - 1 - ((addr / _bytesPerPixel) - (_rowSize * posX));
        posY -= _rowSize - 240;
    }
}

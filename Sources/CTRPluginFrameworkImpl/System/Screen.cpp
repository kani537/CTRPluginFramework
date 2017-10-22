#include "types.h"
#include "3DS.h"

#include "CTRPluginFramework/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include <cstring>
#include <cstdio>
#include "CTRPluginFramework/Utils/Utils.hpp"


namespace CTRPluginFramework
{

    #define REG(x) *(vu32 *)(x)

    static u8  _topBuf[sizeof(ScreenImpl)];
    static u8  _botBuf[sizeof(ScreenImpl)];

    ScreenImpl  *ScreenImpl::Top = nullptr;
    ScreenImpl  *ScreenImpl::Bottom = nullptr;

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

    ScreenImpl::ScreenImpl(u32 lcdSetupInfo, u32 fillColorAddress, bool isTopScreen) :
        _LCDSetup(lcdSetupInfo),
        _FillColor(fillColorAddress),
        _currentBuffer(0), _currentBufferReg((u32 *)(lcdSetupInfo + Select)),
        _width(0), _height(0),
        _stride(0), _rowSize(0), _bytesPerPixel(0),
        _isTopScreen(isTopScreen), _format()
    {
    }

    void    ScreenImpl::Initialize(void)
    {
        ScreenImpl::Top = new (_topBuf) ScreenImpl(SystemImpl::GetIOBasePDC() + 0x400, SystemImpl::GetIOBaseLCD() + 0x204, true);
        ScreenImpl::Bottom = new (_botBuf) ScreenImpl(SystemImpl::GetIOBasePDC() + 0x500, SystemImpl::GetIOBaseLCD() + 0xA04);
    }


    void    ScreenImpl::Fade(float fade, bool copy)
    {
        u32 size = GetFramebufferSize() / _bytesPerPixel;

        u8 *framebuf = (u8 *)_leftFramebuffers[!_currentBuffer];

        PrivColor::SetFormat(_format);
        for (int i = size; i > 0; i--)
        {
            framebuf = PrivColor::ToFramebuffer(framebuf, PrivColor::FromFramebuffer(framebuf).Fade(fade));
        }
    }

    void    ScreenImpl::Acquire(void)
    {
        u32     leftFB1 = FromPhysicalToVirtual(REG(_LCDSetup + FramebufferA1));
        u32     leftFB2 = FromPhysicalToVirtual(REG(_LCDSetup + FramebufferA2));

        _currentBuffer = *_currentBufferReg & 1u;
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

        _leftFramebuffers[0] = leftFB1;
        _leftFramebuffers[1] = leftFB2;

        if (_isTopScreen)
        {
            _rightFramebuffers[0] = leftFB1;
            _rightFramebuffers[1] = leftFB2;

            REG(_LCDSetup + FramebufferB1) = REG(_LCDSetup + FramebufferA1);
            REG(_LCDSetup + FramebufferB2) = REG(_LCDSetup + FramebufferA2);
        }

        Copy();
    }

    void    ScreenImpl::Acquire(u32 left, u32 right, u32 stride, u32 format)
    {
        _currentBuffer = 1;

        _format = (GSPGPU_FramebufferFormats)format;
        _stride = stride;
        _bytesPerPixel = GetBPP(_format);
        _rowSize = _stride / _bytesPerPixel;
        _leftFramebuffers[0] = left;
        _rightFramebuffers[0] = right;

        // Get width & height
        u32 wh = REG(_LCDSetup + LCDSetup::WidthHeight);
        _height = (u16)(wh & 0xFFFF);
        _width = (u16)(wh >> 16);
    }

    void    ScreenImpl::Flush(void)
    {
        u32 size = GetFramebufferSize();
        // Flush currentBuffer
        if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffers[_currentBuffer], size)))
            svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffers[_currentBuffer], size);

        // Flush second buffer
        if (R_FAILED(GSPGPU_FlushDataCache((void *)_leftFramebuffers[!_currentBuffer], size)))
            svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffers[!_currentBuffer], size);

        if (Is3DEnabled())
        {
            if (R_FAILED(GSPGPU_FlushDataCache((void *)_rightFramebuffers[_currentBuffer], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffers[_currentBuffer], size);

            // Flush second buffer
            if (R_FAILED(GSPGPU_FlushDataCache((void *)_rightFramebuffers[!_currentBuffer], size)))
                svcFlushProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffers[!_currentBuffer], size);      
        }
    }

	void	ScreenImpl::Invalidate(void)
	{
		u32 size = GetFramebufferSize();

		// Invalidate currentBuffer
		if (R_FAILED(GSPGPU_InvalidateDataCache((void *)_leftFramebuffers[_currentBuffer], size)))
			svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffers[_currentBuffer], size);

    	// Invalidate second buffer
		if (R_FAILED(GSPGPU_InvalidateDataCache((void *)_leftFramebuffers[!_currentBuffer], size)))
			svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffers[!_currentBuffer], size);

		if (Is3DEnabled())
		{
			if (R_FAILED(GSPGPU_InvalidateDataCache((void *)_rightFramebuffers[_currentBuffer], size)))
				svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffers[_currentBuffer], size);

			// Invalidate second buffer
			if (R_FAILED(GSPGPU_InvalidateDataCache((void *)_rightFramebuffers[!_currentBuffer], size)))
				svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffers[!_currentBuffer], size);
		}
	}

	void    ScreenImpl::Copy(void)
    {
        u32 size = GetFramebufferSize();

        Flush();
        Invalidate();

        // Copy current buffer in the other one
        memcpy((void *)_leftFramebuffers[!_currentBuffer], (void *)_leftFramebuffers[_currentBuffer], size);
    }

    bool    ScreenImpl::IsTopScreen(void)
    {
        return (_isTopScreen);
    }

    bool    ScreenImpl::Is3DEnabled(void)
    {
        if (!_isTopScreen)
            return (false);

        u32 left = _leftFramebuffers[!_currentBuffer];
        u32 right = _rightFramebuffers[!_currentBuffer];

        return (right && right != left && *(float *)(0x1FF81080) > 0.f);
    }

    void    ScreenImpl::Flash(Color &color)
    {
        u32     fillColor = (color.ToU32() & 0xFFFFFF) | 0x01000000;

        for (int i = 0; i < 0x64; i++)
        {
            REG(_FillColor) = fillColor;
            svcSleepThread(5000000); // 0.005 second
        }
        REG(_FillColor) = 0;
    }

    void    ScreenImpl::Clean(void)
    {
        GSPGPU_RestoreVramSysArea();
        GSPGPU_SaveVramSysArea();
        Top->Acquire();
        Bottom->Acquire();
    }

    /*
    ** Swap buffers
    *****************/

    void    ScreenImpl::SwapBuffer(bool flush, bool copy)
    {   
        if (flush)
            Flush();
        
        // Change buffer
        _currentBuffer = !_currentBuffer;

        // Set gpu buffer
        if (!_currentBuffer)
            *_currentBufferReg &= ~1u;
        else
            *_currentBufferReg |= 1u;

        // Ensure rights framebuffers
        if (_isTopScreen)
        {
            REG(_LCDSetup + FramebufferB1) = REG(_LCDSetup + FramebufferA1);
            REG(_LCDSetup + FramebufferB2) = REG(_LCDSetup + FramebufferA2);
        }

        if (copy)
            Copy();
    }

    GSPGPU_FramebufferFormats   ScreenImpl::GetFormat(void)
    {
        return (_format);
    }

    u16     ScreenImpl::GetWidth(void)
    {
        return (_width);
    }

    u16     ScreenImpl::GetHeight(void)
    {
        return (_height);
    }

    u32     ScreenImpl::GetStride(void)
    {
        return (_stride);
    }

    u32     ScreenImpl::GetRowSize(void)
    {
        return (_rowSize);
    }

    u32     ScreenImpl::GetBytesPerPixel(void)
    {
        return (_bytesPerPixel);
    }

    u32     ScreenImpl::GetFramebufferSize(void)
    {
        return (_stride * _width);
    }

    void    ScreenImpl::GetFramebufferInfos(int &rowstride, int &bpp, GSPGPU_FramebufferFormats &format)
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
    u8      *ScreenImpl::GetLeftFramebuffer(bool current)
    {
        return ((u8 *)_leftFramebuffers[ current ? _currentBuffer : !_currentBuffer ]);            
    }

    u8      *ScreenImpl::GetLeftFramebuffer(int posX, int posY)
    {

        posX = std::max(posX, 0);
        posX = std::min(posX, (_isTopScreen ? 400 : 320));
        posY = std::max(posY, 0);
        posY = std::min(posY, 240);

        // Correct posY
        posY += _rowSize - 240;

        u32 offset = (_rowSize - 1 - posY + posX * _rowSize) * _bytesPerPixel;

        return ((u8 *)_leftFramebuffers[!_currentBuffer] + offset);            
    }

    /*
    ** Right
    *************/

    u8      *ScreenImpl::GetRightFramebuffer(bool current)
    {
        if (!_isTopScreen)
            return (nullptr);

        return ((u8 *)_rightFramebuffers[ current ? _currentBuffer : !_currentBuffer ]);            
    }

    u8      *ScreenImpl::GetRightFramebuffer(int posX, int posY)
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

        return ((u8 *)_rightFramebuffers[!_currentBuffer] + offset);            
    }

    void    ScreenImpl::GetPosFromAddress(u32 addr, int &posX, int &posY)
    {
        addr -= _leftFramebuffers[!_currentBuffer];

        posX = addr / (_rowSize * _bytesPerPixel);
        posY = _rowSize - 1 - ((addr / _bytesPerPixel) - (_rowSize * posX));
        posY -= _rowSize - 240;
    }
}

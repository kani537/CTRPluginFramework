#include "types.h"

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFramework/System/System.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "ctrulib/allocator/vram.h"


namespace CTRPluginFramework
{
    #define REG(x) *(vu32 *)(x)

    // Reserve the place for the Screen objects
    static u8  _topBuf[sizeof(ScreenImpl)];
    static u8  _botBuf[sizeof(ScreenImpl)];

    ScreenImpl  *ScreenImpl::Top = nullptr;
    ScreenImpl  *ScreenImpl::Bottom = nullptr;

    static  inline  void memcpy32(u32 *dst, u32 *src, u32 size)
    {
        for (; size > 0; size -= 4)
            *dst++ = *src++;
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

    ScreenImpl::ScreenImpl(u32 lcdSetupInfo, u32 fillColorAddress, bool isTopScreen) :
        _LCDSetup(lcdSetupInfo),
        _FillColor(fillColorAddress),
        _currentBufferReg((u32 *)(lcdSetupInfo + Select)),
        _leftFramebuffers{},
        _rightFramebuffers{},
        _backupFramebuffer{ nullptr },
        _currentBuffer(0),
        _width(0), _height(0),
        _stride(0), _rowSize(0), _bytesPerPixel(0),
        _isTopScreen(isTopScreen), _format()
    {
    }

    void    ScreenImpl::Initialize(void)
    {
        ScreenImpl::Top = new (_topBuf) ScreenImpl(SystemImpl::IoBasePDC + 0x400, SystemImpl::IoBaseLCD + 0x204, true);
        ScreenImpl::Bottom = new (_botBuf) ScreenImpl(SystemImpl::IoBasePDC + 0x500, SystemImpl::IoBaseLCD + 0xA04);
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

        // Alloc framebuffer
        u32 size = GetFramebufferSize();
        if (_backupFramebuffer == nullptr)
        {
            _backupFramebuffer = static_cast<u32 *>(vramAlloc(size));
            if (_backupFramebuffer == nullptr)
            {
                //OSD::Notify("Failure");
                return;
            }
        }

        // Invalidate cache
        //Invalidate();

        // Backup the framebuffer
        memcpy32(_backupFramebuffer, (u32 *)GetLeftFramebuffer(), size);

        // Copy the framebuffer to the second framebuffer (avoid the sensation of flickering on buffer swap)
        memcpy32((u32 *)GetLeftFramebuffer(true), (u32 *)GetLeftFramebuffer(), size);
    }

    void    ScreenImpl::Acquire(u32 left, u32 right, u32 stride, u32 format, bool backup)
    {
        _currentBuffer = 1;

        _format = (GSPGPU_FramebufferFormats)(format & 7);
        _stride = stride;
        _bytesPerPixel = GetBPP(_format);
        _rowSize = _stride / _bytesPerPixel;
        _leftFramebuffers[0] = left;
        _rightFramebuffers[0] = right;

        // Get width & height
        u32 wh = REG(_LCDSetup + LCDSetup::WidthHeight);
        _height = (u16)(wh & 0xFFFF);
        _width = (u16)(wh >> 16);

        if (backup && _backupFramebuffer != nullptr)
            memcpy32(_backupFramebuffer, (u32 *)left, stride * _width);
    }

    void    ScreenImpl::Flush(void)
    {
        u32 size = GetFramebufferSize();

        // Flush currentBuffer
        svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffers[_currentBuffer], size);

        // Flush second buffer
        svcFlushProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffers[!_currentBuffer], size);

        if (Is3DEnabled())
        {
            // Flush current buffer
            svcFlushProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffers[_currentBuffer], size);

            // Flush second buffer
            svcFlushProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffers[!_currentBuffer], size);
        }
    }

	void	ScreenImpl::Invalidate(void)
	{
		u32 size = GetFramebufferSize();

		// Invalidate currentBuffer
		svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffers[_currentBuffer], size);

    	// Invalidate second buffer
		svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_leftFramebuffers[!_currentBuffer], size);

		if (Is3DEnabled())
		{
            // Invalidate current buffer
			svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffers[_currentBuffer], size);

			// Invalidate second buffer
		    svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_rightFramebuffers[!_currentBuffer], size);
		}
	}

	void    ScreenImpl::Copy(void)
    {
        u32 size = GetFramebufferSize();

        // Copy the framebuffer to the second framebuffer (avoid the sensation of flickering on buffer swap)
        memcpy32((u32 *)GetLeftFramebuffer(), (u32 *)GetLeftFramebuffer(true), size);
    }

    void    ScreenImpl::Debug(void)
    {
        /*
        int posY = 10;
        if (_isTopScreen)
        {
            Renderer::SetTarget(TOP);
            Renderer::DrawString(Utils::Format("FB0: %08X", _leftFramebuffers[0]).c_str(), 10, posY, Color::Blank, Color::Black);
            Renderer::DrawString(Utils::Format("FB1: %08X", _leftFramebuffers[1]).c_str(), 10, posY, Color::Blank, Color::Black);
            Renderer::DrawString(Utils::Format("Sel: %d", _originalBuffer).c_str(), 10, posY, Color::Blank, Color::Black);
        }
        else
        {
            Renderer::SetTarget(BOTTOM);
            Renderer::DrawString(Utils::Format("FB0: %08X", _leftFramebuffers[0]).c_str(), 10, posY, Color::Blank, Color::Black);
            Renderer::DrawString(Utils::Format("FB1: %08X", _leftFramebuffers[1]).c_str(), 10, posY, Color::Blank, Color::Black);
            Renderer::DrawString(Utils::Format("Sel: %d", _originalBuffer).c_str(), 10, posY, Color::Blank, Color::Black);
        }
        */
    }

    bool    ScreenImpl::IsTopScreen(void)
    {
        return (_isTopScreen);
    }

    using Pixel = BMPImage::Pixel;

    void    ScreenToBMP_BGR8(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride)
    {
        u32     height = 240;

        stride -= 3;

        while (height--)
        {
            u8 *fb = src;

            for (u32 w = width; w > 0; --w, ++bmp)
            {
                bmp->b = *fb++;
                bmp->g = *fb++;
                bmp->r = *fb++;
                fb += stride;
            }
            src += 3;
            bmp += padding;
        }
    }

    void    ScreenToBMP_RGBA8(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride)
    {
        u32     height = 240;

        stride -= 3;

        while (height--)
        {
            u8 *fb = src + 1; //:< Skip first alpha component

            for (u32 w = width; w > 0; --w, ++bmp)
            {
                bmp->b = *fb++;
                bmp->g = *fb++;
                bmp->r = *fb++;
                fb += stride;
            }
            src += 4;
            bmp += padding;
        }
    }

    void    ScreenToBMP_RGB565(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride)
    {
        u32     height = 240;

        u16     *src16 = (u16 *)src;

        stride >>= 1;

        while (height--)
        {
            u16 *fb = src16;

            for (u32 w = width; w > 0; --w, ++bmp)
            {
                const u16 c = *fb;

                bmp->b = (c << 3) & 0xF8;
                bmp->g = (c >> 3) & 0xFC;
                bmp->r = (c >> 8) & 0xF8;
                fb += stride;
            }
            ++src16;
            bmp += padding;
        }
    }

    void    ScreenToBMP_RGB5A1(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride)
    {
        u32     height = 240;

        u16     *src16 = (u16 *)src;

        stride >>= 1;

        while (height--)
        {
            u16 *fb = src16;

            for (u32 w = width; w > 0; --w, ++bmp)
            {
                const u16 c = *fb;

                bmp->b = (c << 2) & 0xF8;
                bmp->g = (c >> 3) & 0xF8;
                bmp->r = (c >> 8) & 0xF8;

                fb += stride;
            }
            ++src16;
            bmp += padding;
        }
    }

    void    ScreenToBMP_RGBA4(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride)
    {
        u32     height = 240;

        u16     *src16 = (u16 *)src;

        stride >>= 1;

        while (height--)
        {
            u16 *fb = src16;

            for (u32 w = width; w > 0; --w, ++bmp)
            {
                const u16 c = *fb;

                bmp->b = c & 0xF0;
                bmp->g = (c >> 4) & 0xF0;
                bmp->r = (c >> 8) & 0xF0;
                fb += stride;
            }
            ++src16;
            bmp += padding;
        }
    }

    void    ScreenImpl::ScreenToBMP(Pixel *bmp, u32 padding)
    {
        if (bmp == nullptr)
            return;

        u8      *src = reinterpret_cast<u8 *>(_backupFramebuffer);

        if (src == nullptr)
            src = GetLeftFramebuffer();

        if (_format == GSP_RGBA8_OES) return ScreenToBMP_RGBA8(bmp, padding, src, _width, _stride);
        if (_format == GSP_BGR8_OES) return ScreenToBMP_BGR8(bmp, padding, src, _width, _stride);
        if (_format == GSP_RGB565_OES) return ScreenToBMP_RGB565(bmp, padding, src, _width, _stride);
        if (_format == GSP_RGB5_A1_OES) return ScreenToBMP_RGB5A1(bmp, padding, src, _width, _stride);
        if (_format == GSP_RGBA4_OES) return ScreenToBMP_RGBA4(bmp, padding, src, _width, _stride);
    }

    static inline BMPImage* CreateBMP(u32 width, u32 height)
    {
        return new BMPImage(width, height, false);
    }

    BMPImage *ScreenImpl::Screenshot(int screen, BMPImage *image)
    {
        BMPImage    *bmp = image;

        // Top screen only
        if (screen == SCREENSHOT_TOP)
        {
            if (bmp == nullptr)
                bmp = CreateBMP(400, 240);

            Top->ScreenToBMP(reinterpret_cast<BMPImage::Pixel *>(bmp->data()));
        }
        // Bottom screen only
        else if (screen == SCREENSHOT_BOTTOM)
        {
            if (bmp == nullptr)
                bmp = CreateBMP(320, 240);
            Bottom->ScreenToBMP(reinterpret_cast<BMPImage::Pixel *>(bmp->data()));
        }
        // Both screens
        else
        {
            if (bmp == nullptr)
                bmp = CreateBMP(400, 480);

            // Bottom screen comes first in the bmp
            BMPImage::Pixel *dst = reinterpret_cast<BMPImage::Pixel *>(bmp->data());

            Bottom->ScreenToBMP(dst + 40, 80);

            // Then the top screen
            dst += 400 * 240;
            Top->ScreenToBMP(dst);
        }

        return bmp;
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
        if (!System::IsNew3DS())
            return;

        __dsb();

        u32 *src = Top->_backupFramebuffer;

        if (src != nullptr)
        {
            for (int i = 0; i < 2; ++i)
                memcpy32((u32 *)Top->_leftFramebuffers[i], src, Top->GetFramebufferSize());
        }

        src = Bottom->_backupFramebuffer;

        if (src != nullptr)
        {
            for (int i = 0; i < 2; ++i)
                memcpy32((u32 *)Bottom->_leftFramebuffers[i], src, Bottom->GetFramebufferSize());
        }

        Top->Flush();
        Bottom->Flush();
    }

    void    ScreenImpl::ApplyFading(void)
    {
        Top->Fade(0.5f);
        Bottom->Fade(0.5f);

        __dsb();

        Top->SwapBuffer(true, true);
        Bottom->SwapBuffer(true, true);
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
        {
            if (IsTopScreen())
                gspWaitForVBlank1();
            else
                gspWaitForVBlank();
            Copy();
        }
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

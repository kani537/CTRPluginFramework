#include "NTR.hpp"
#include "NTRImpl.hpp"
#include "3DS.h"
#include "CTRPluginFramework/System/Process.hpp"
#include <algorithm>
#include "CTRPluginFramework/Menu/PluginMenu.hpp"
#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"
#include "font6x10Linux.h"

#define CALLBACK_OVERLAY (101)
#define NS_CONFIGURE_ADDR	0x06000000
struct NS_CONFIG {
    u32 initMode;
    u32 startupCommand;
    u32 hSOCU;

    u8* debugBuf;
    u32 debugBufSize;
    u32 debugPtr;
    u32 debugReady;

    //RT_LOCK debugBufferLock;
    vu32 debugBufferLock;

    u32 startupInfo[32];
    u32 allowDirectScreenAccess;
    u32 exitFlag;

    u32 sharedFunc[100];

};

extern "C" u32 plgRegisterCallback(u32 type, void* callback, u32 param0);


namespace CTRPluginFramework
{
    bool    NTRImpl::IsOSDAvailable = false;
    bool    NTRImpl::MessColors = false;
    OSDParams NTRImpl::OSDParameters = { 0 };

    u32 rtGenerateJumpCode(u32 dst, u32* buf) {
        buf[0] = 0xe51ff004;
        buf[1] = dst;
        return 8;
    }

    u32     GetBPP(GSPGPU_FramebufferFormats format);
    u32 g_framebuffer = 0;

    void    MessColor(u32 startAddr, u32 stride, u32 format)
    {
        u32 endBuffer = startAddr + (stride * 400);
        u32 bpp = GetBPP((GSPGPU_FramebufferFormats)format);

        PrivColor::SetFormat((GSPGPU_FramebufferFormats)format);

        if (bpp == 4)
        {

            for (int x = 0; x < 400; ++x)
            {
                u32 *fb = (u32 *)(startAddr + stride * x);
                u32 fbend = (u32)fb + 240 * 4;

                while (fb < (u32 *)fbend)
                {
                    u32 c = *fb;

                    //color.a = 255;
                    u8 b = (c >> 16); //Swap R
                    //u8 g = (c >> 8);
                    u8 r = c; //Swap b

                              //color.Fade(0.1f);

                    c &= 0xFF00FF00;
                    c |= r << 16;
                    c |= b;

                    *fb++ = c;
                }
            }
        }

        else if (bpp == 3)
        {
            for (int x = 0; x < 400; ++x)
            {
                u32 *fb = (u32 *)(startAddr + stride * x);
                u32 fbend = (u32)fb + 240 * 3;

                while (fb < (u32 *)fbend)
                {
                    u32 c = *fb;

                    u8 b = (c >> 16); //Swap R
                    u8 g = (c >> 8);
                    u8 r = c; //Swap b

                    c &= 0xFF000000;
                    c |= r << 16;
                    c |= g << 8;
                    c |= b;

                    *fb = c;
                    fb = (u32 *)((u32)fb + 3);
                }
            }
        }
        else if (bpp == 2)
        {
            for (int x = 0; x < 400; ++x)
            {                
                u16 *fb = (u16 *)(startAddr + stride * x);
                u16 *fbEnd = fb + 240;

                while (fb < fbEnd)
                {
                    u16 c = *fb;

                    u8 b = (c >> 8) & 0xF8; //Swap R
                    u8 g = (c >> 3) & 0xFC;
                    u8 r = (c << 3) & 0xF8; //Swap b

                    c = (r & 0xF8) << 8;
                    c |= (g & 0xFC) << 3;
                    c |= (b & 0xF8) >> 3;

                    *fb++ = c;
                }
            }            
        }
    }

    u32    MainOverlayCallback(u32 isBottom, u32 addr, u32 addrB, u32 stride, u32 format)
    {        
        // OSD is available
        NTRImpl::IsOSDAvailable = true;
        static u32 lastAddr = 0;

        if (addr == lastAddr)
            return(1);

        // Set OSDParams
        OSDParams &params = NTRImpl::OSDParameters;

        params.isBottom = isBottom;
        params.is3DEnabled = *(float *)(0x1FF81080) > 0.f && !isBottom && addr != addrB && addrB;
        params.leftFramebuffer = addr;
        params.rightFramebuffer = addrB;
        params.stride = stride;
        params.format = format & 0xf;
        params.screenWidth = isBottom ? 320 : 400;

        PluginMenu *menu = PluginMenu::GetRunningInstance();

        if (isBottom || menu == nullptr || menu->IsOpen() || addr == 0)
            return (1);
        
        lastAddr = addr;
        

        bool    isFbModified = false;

        if (NTRImpl::MessColors)
        {
            isFbModified = true;
            MessColor(addr, stride, format & 0xf);
            if (params.is3DEnabled)
                MessColor(addrB, stride, format & 0xf);
        }

        // Execute OSD
        isFbModified |= OSDImpl::GetInstance()->Draw();

        return (!isFbModified);
    }

    void    NTRImpl::InitNTR(void)
    {
        rtGenerateJumpCode(((NS_CONFIG *)(NS_CONFIGURE_ADDR))->sharedFunc[5], (u32 *)plgRegisterCallback);
        svcFlushProcessDataCache(Process::GetHandle(), (void*)plgRegisterCallback, 8);

        plgRegisterCallback(CALLBACK_OVERLAY, (void *)MainOverlayCallback, 0);
    }

    bool    NTR::IsOSDAvailable(void)
    {
        return (NTRImpl::IsOSDAvailable);
    }

    void    NTR::NewCallback(OverlayCallback callback)
    {
        plgRegisterCallback(CALLBACK_OVERLAY, (void *)callback, 0);
    }

    void    NTR::FetchOSDParams(OSDParams& parameters)
    {
        parameters = NTRImpl::OSDParameters;
    }

    u32     NTR::GetLeftFramebuffer(u32 posX, u32 posY)
    {
        OSDParams &params = NTRImpl::OSDParameters;

        u32 bpp = GetBPP((GSPGPU_FramebufferFormats)params.format);
        u32 rowsize = params.stride / bpp;

        posX = std::min(posX, (params.isBottom ? (u32)320 : (u32)400));
        posY = std::min(posY, (u32)240);

        // Correct posY
        posY += rowsize - 240;

        u32 offset = (rowsize - 1 - posY + posX * rowsize) * bpp;

       /* 512
        514
        492 // Nothing
        460 // Top

        460 + (10 * 2) = 480 
        */

        return (params.leftFramebuffer + offset);
        /*
        int sposY = (int)posY - rowsize - 240;

        u32 offset = params.stride * posX + 240 * bpp - bpp * sposY;

        return (params.leftFramebuffer + offset); */
    }

    u32     NTR::GetRightFramebuffer(u32 posX, u32 posY)
    {
        OSDParams &params = NTRImpl::OSDParameters;

        u32 bpp = GetBPP((GSPGPU_FramebufferFormats)params.format);
        u32 rowsize = params.stride / bpp;

        //posY -= rowsize - 240;

        posX = std::min(posX, (params.isBottom ? (u32)320 : (u32)400));
        posY = std::min(posY, (u32)240);

        u32 offset = params.stride * posX + 240 * bpp - bpp * posY;

        return (params.rightFramebuffer + offset);
    }

    /*
    {
    for (int x = -2; x < 0; x++)
    {
    for (int i = 0; i < 10; i++)
    {
    u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX + x, posY + i);
    u8  *framebuf1 = (u8 *)NTR::GetRightFramebuffer(posX + x - offset, posY + i);
    PrivColor::ToFramebuffer(framebuf0, bg);
    PrivColor::ToFramebuffer(framebuf1, bg);
    }
    }

    }

     */

    static int     DrawCharacter(int posX, int posY, char c, const Color &fg)
    {
        int index = c * 10;
        u32 stride = NTRImpl::OSDParameters.stride;

        if (NTRImpl::OSDParameters.is3DEnabled)
        {               
            for (int yy = 0; yy < 10; yy++)
            {
                u8 charPos = font[index + yy];

                int x = 0;
                u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX, posY + yy);
                u8  *framebuf1 = (u8 *)NTR::GetRightFramebuffer(posX - 10, posY + yy);

                for (int xx = 6; xx >= 0; xx--, x++)
                {
                    if ((charPos >> xx) & 1)
                    {
                        PrivColor::ToFramebuffer(framebuf0, fg);
                        PrivColor::ToFramebuffer(framebuf1, fg);
                    }
                    framebuf0 += stride;
                    framebuf1 += stride;
                }
            }
        }
        // No 3D
        else
        {
            for (int yy = 0; yy < 10; yy++)
            {
                u8 charPos = font[index + yy];

                int x = 0;
                u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX, posY + yy);

                for (int xx = 6; xx >= 0; xx--, x++)
                {
                    if ((charPos >> xx) & 1)
                        PrivColor::ToFramebuffer(framebuf0, fg);
                    framebuf0 += stride;
                }
            }
        }
        return (posX + 6);
    }

    static int     DrawCharacter(int posX, int posY, char c, const Color &fg, const Color &bg)
    {
        int index = c * 10;
        u32 stride = NTRImpl::OSDParameters.stride;

        if (NTRImpl::OSDParameters.is3DEnabled)
        {
            for (int yy = 0; yy < 10; yy++)
            {
                u8 charPos = font[index + yy];

                int x = 0;
                u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX, posY + yy);
                u8  *framebuf1 = (u8 *)NTR::GetRightFramebuffer(posX - 10, posY + yy);

                for (int xx = 6; xx >= 0; xx--, x++)
                {
                    if ((charPos >> xx) & 1)
                    {
                        PrivColor::ToFramebuffer(framebuf0, fg);
                        PrivColor::ToFramebuffer(framebuf1, fg);
                    }
                    else
                    {
                        PrivColor::ToFramebuffer(framebuf0, bg);
                        PrivColor::ToFramebuffer(framebuf1, bg);
                    }
                    framebuf0 += stride;
                    framebuf1 += stride;
                }
            }
        }
        // No 3D
        else
        {
            for (int yy = 0; yy < 10; yy++)
            {
                u8 charPos = font[index + yy];

                int x = 0;
                u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX, posY + yy);

                for (int xx = 6; xx >= 0; xx--, x++)
                {
                    if ((charPos >> xx) & 1)
                        PrivColor::ToFramebuffer(framebuf0, fg);
                    else
                        PrivColor::ToFramebuffer(framebuf0, bg);
                    framebuf0 += stride;
                }
            }
        }
        return (posX + 6);
    }

    int     Draw::String(int posX, int posY, const Color& foreground, const u8* str)
    {
        if (!str || !*str)
            return (posY);

        char    c;
        int     posXX = posX;
        OSDParams &params = NTRImpl::OSDParameters;

        while ((c = *str++) != 0)
        {
            if (posX + 6 >= params.screenWidth)
                break;

            if (c == '\n')
            {
                posXX = posX;
                posY += 10;
                continue;
            }

            posXX = DrawCharacter(posXX, posY, c, foreground);
        }

        return (posY + 10);
    }

    int Draw::String(int posX, int posY, const Color& foreground, const Color& background, const u8* str)
    {
        if (!str || !*str)
            return (posY);

        char    c;
        int     posXX = posX;
        OSDParams &params = NTRImpl::OSDParameters;

        for (int x = -2; x < 0; x++)
        {
            for (int i = 0; i < 10; i++)
            {
                u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX + x, posY + i);
                PrivColor::ToFramebuffer(framebuf0, background);

                if (params.is3DEnabled)
                {
                    u8  *framebuf1 = (u8 *)NTR::GetRightFramebuffer(posX + x - 10, posY + i);
                    PrivColor::ToFramebuffer(framebuf1, background);
                }
            }
        }

        while ((c = *str++) != 0)
        {
            if (posX + 6 >= params.screenWidth)
                break;

            if (c == '\n')
            {
                posXX = posX;
                posY += 10;
                continue;
            }

            posXX = DrawCharacter(posXX, posY, c, foreground, background);
        }

        return (posY + 10);
    }
}

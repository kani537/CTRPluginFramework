#include "NTR.hpp"
#include "NTRImpl.hpp"
#include "3DS.h"
#include "CTRPluginFramework/System/Process.hpp"
#include <algorithm>
#include "CTRPluginFramework/Menu/PluginMenu.hpp"
#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"
#include "font6x10Linux.h"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "CTRPluginFramework/System/Sleep.hpp"

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

namespace CTRPluginFramework
{
    bool    NTRImpl::IsOSDAvailable = true;
    bool    NTRImpl::MessColors = false;

    u32     GetBPP(GSPGPU_FramebufferFormats format);

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
}

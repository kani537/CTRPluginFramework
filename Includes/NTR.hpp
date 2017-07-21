#ifndef NTR_HPP
#define NTR_HPP

#include "types.h"

namespace CTRPluginFramework
{
    struct OSDParams
    {
        bool    isBottom;
        bool    is3DEnabled;
        u32     leftFramebuffer;
        u32     rightFramebuffer;
        u32     stride;
        u32     format;
        u32     screenWidth;
    };

    class NTR
    {
    public:
        using OverlayCallback = u32(*)(u32 isBottom, u32 addr, u32 addrB, u32 stride, u32 format);

        static bool    IsOSDAvailable(void);
        static void    NewCallback(OverlayCallback callback);
        static void    FetchOSDParams(OSDParams &parameters);
        static u32     GetLeftFramebuffer(u32 posX, u32 posY);
        static u32     GetRightFramebuffer(u32 posX, u32 posY);
    };
}

#endif
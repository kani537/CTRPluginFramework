#include "NTR.hpp"
#include "NTRImpl.hpp"
#include "3DS.h"
#include "CTRPluginFramework/System/Process.hpp"
#include <algorithm>
#include "CTRPluginFramework/Menu/PluginMenu.hpp"
#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"

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
    OSDParams NTRImpl::OSDParameters = { 0 };

    u32 rtGenerateJumpCode(u32 dst, u32* buf) {
        buf[0] = 0xe51ff004;
        buf[1] = dst;
        return 8;
    }

    u32    MainOverlayCallback(u32 isBottom, u32 addr, u32 addrB, u32 stride, u32 format)
    {
        
        // OSD is available
        NTRImpl::IsOSDAvailable = true;

        PluginMenu *menu = PluginMenu::GetRunningInstance();

        if (menu == nullptr || menu->IsOpen() || addr == 0 || isBottom)
            return (1);

        // Set OSDParams
        OSDParams &params = NTRImpl::OSDParameters;

        params.isBottom = isBottom;
        params.is3DEnabled = *(float *)(0x1FF81080) > 0.f && !isBottom && addr != addrB && addrB;
        params.leftFramebuffer = addr;
        params.rightFramebuffer = addrB;
        params.stride = stride;
        params.format = format;
        params.screenWidth = isBottom ? 320 : 400;

        bool    isFbModified = false;

        // Execute OSD
        OSDImpl *osd = OSDImpl::GetInstance();

        isFbModified = osd->Draw();

        return (!isFbModified);
    }

    void    NTRImpl::InitNTR(void)
    {
        rtGenerateJumpCode(((NS_CONFIG *)(NS_CONFIGURE_ADDR))->sharedFunc[5], (u32 *)plgRegisterCallback);
        svcFlushProcessDataCache(Process::GetHandle(), (void*)plgRegisterCallback, 8);

        plgRegisterCallback(CALLBACK_OVERLAY, (void *)MainOverlayCallback, 0);
    }

    u32 GetBPP(GSPGPU_FramebufferFormats format);

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

    u32     NTR::GetLeftFramebuffer(int posX, int posY)
    {
        OSDParams &params = NTRImpl::OSDParameters;

        u32 bpp = GetBPP((GSPGPU_FramebufferFormats)params.format);
        u32 rowsize = params.stride / bpp;

        posX = std::max(posX, 0);
        posX = std::min(posX, (params.isBottom ? 320 : 400));
        posY = std::max(posY, 0);
        posY = std::min(posY, 240);

        // Correct posY
        //posY += rowsize - 240;
        u32 offset = (rowsize - 1 - posY + posX * rowsize) * bpp;

        return (params.leftFramebuffer + params.stride * posX + 240 * bpp - bpp * posY);
        return (params.leftFramebuffer + offset);
    }

    u32     NTR::GetRightFramebuffer(int posX, int posY)
    {
        OSDParams &params = NTRImpl::OSDParameters;

        u32 bpp = GetBPP((GSPGPU_FramebufferFormats)params.format);
        u32 rowsize = params.stride / bpp;

        posX = std::max(posX, 0);
        posX = std::min(posX, (params.isBottom ? 320 : 400));
        posY = std::max(posY, 0);
        posY = std::min(posY, 240);

        // Correct posY
        posY += rowsize - 240;
        u32 offset = (rowsize - 1 - posY + posX * rowsize) * bpp;

        return (params.rightFramebuffer + offset);
    }
}

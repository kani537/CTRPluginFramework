#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "font6x10Linux.h"

#include <vector>
#include <cstring>
#include "CTRPluginFramework/System/Sleep.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "CTRPluginFramework/System/Hook.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"
#include "CTRPluginFramework/System/Touch.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuImpl.hpp"
#include "CTRPluginFrameworkImpl/System/Screenshot.hpp"

namespace CTRPluginFramework
{
    bool        OSDImpl::DrawSaveIcon = false;
    bool        OSDImpl::MessColors = false;
    u32         OSDImpl::WaitingForScreenshot = 0;
    u32         OSDImpl::FramesToPlay = 0;
    OSDReturn   OSDImpl::HookReturn = nullptr;
    Hook        OSDImpl::OSDHook;
    Screen      OSDImpl::TopScreen;
    Screen      OSDImpl::BottomScreen;
    RecursiveLock OSDImpl::RecLock;
    FloatingButton OSDImpl::FloatingBtn(IntRect(0, 0, 40, 40), Icon::DrawRocket);
    std::list<OSDImpl::OSDMessage*> OSDImpl::Notifications;
    std::vector<OSDCallback> OSDImpl::Callbacks;

    bool        OSDImpl::IsFramePaused = false;
    LightEvent  OSDImpl::OnNewFrameEvent;
    LightEvent  OSDImpl::OnFramePaused;
    LightEvent  OSDImpl::OnFrameResume;

    void    InstallOSD(void);

    void    OSDImpl::_Initialize(void)
    {
        RecursiveLock_Init(&RecLock);

        // Init frame event
        LightEvent_Init(&OnNewFrameEvent, RESET_STICKY);
        LightEvent_Init(&OnFramePaused, RESET_STICKY);
        LightEvent_Init(&OnFrameResume, RESET_STICKY);
        IsFramePaused = false;

        InstallOSD();
    }

#define XEND    390

    OSDImpl::OSDMessage::OSDMessage(const std::string& str, const Color& fg, const Color& bg)
    {
        text = str;
        width = Renderer::LinuxFontSize(text.c_str());
        drawn = false;
        foreground = fg;
        background = bg;
        time = Clock();
    }

    void    OSDImpl::Update(void)
    {
        if (Preferences::UseFloatingBtn)
        {
            FloatingBtn.Update(Touch::IsDown(), IntVector(Touch::GetPosition()));
            if (FloatingBtn())
                PluginMenuImpl::ForceOpen();
        }

        if (TryLock())
            return;

        while (Notifications.size() && Notifications.front()->drawn)
        {
            OSDMessage *message = Notifications.front();

            if (message->time.HasTimePassed(Seconds(5.f)))
            {
                delete message;
                Notifications.pop_front();
            }
            else
                break;
        }

        Unlock();
    }

    bool    OSDImpl::Draw(void)
    {
        Lock();

        if (Notifications.empty())
        {
            Unlock();
            return (false);
        }

        int posX;
        int posY = std::min((u32)15, (u32)Notifications.size());
        posY = 230 - (15 * posY);
        int count = 0;

        for (OSDMessage *message : Notifications)
        {
            posX = XEND - message->width;
            Renderer::DrawString(message->text.c_str(), posX, posY, message->foreground, message->background);
            posY += 5;
            if (!message->drawn)
                message->time.Restart();

            message->drawn = true;

            count++;
            if (count >= 15)
                break;
        }

        Unlock();
        return (true);
    }

    void    OSDImpl::Lock(void)
    {
        RecursiveLock_Lock(&RecLock);
    }

    bool OSDImpl::TryLock(void)
    {
        return (RecursiveLock_TryLock(&RecLock));
    }

    void    OSDImpl::Unlock(void)
    {
        RecursiveLock_Unlock(&RecLock);
    }

    u32 GetBPP(GSPFormat format);

    static const float  g_second = Seconds(1.f).AsSeconds();
    static Clock        g_fpsClock[2];

    static void    MessColor(u32 startAddr, u32 stride, u32 format);

    int     OSDImpl::MainCallback(u32 isBottom, int arg2, void *addr, void *addrB, int stride, int format, int arg7)
    {
        if (addr)
        {
            CallbackGlobal(isBottom, addr, addrB, stride, format);
        }
        return (HookReturn(isBottom, arg2, addr, addrB, stride, format, arg7));
    }

    int     OSDImpl::MainCallback2(u32 r0, u32 *params, u32 isBottom, u32 arg)
    {
        using OSDReturn = int(*)(u32, u32 *, u32, u32);

        u32 addr, addrB, stride, format;

        if (!params)
            goto exit;
       // isBottom = params[0];
        addr = params[1];
        addrB = 0;
        //addrB = params[2];
        stride = params[3];
        format = params[4] & 0xF;

        if (addr)
            CallbackGlobal(isBottom, (void *)addr, (void *)addrB, stride, format);

    exit:
        return ((OSDReturn)(OSDImpl::HookReturn))(r0, params, isBottom, arg);
    }

    void     OSDImpl::CallbackGlobal(u32 isBottom, void* addr, void* addrB, int stride, int format)
    {
        if (Screenshot::OSDCallback(isBottom, addr, addrB, stride, format))
            return;

        if (!isBottom)
        {
            if (FramesToPlay)
                --FramesToPlay;

            // Signal a new frame to all threads waiting for it
            LightEvent_Pulse(&OnNewFrameEvent);
        }

        // If frame have to be paused
        if (!WaitingForScreenshot && !FramesToPlay && ProcessImpl::_isPaused)
        {
            __dsb();

            IsFramePaused = true;

            // Wake up threads waiting for frame to be paused
            LightEvent_Pulse(&OnFramePaused);

            // Wait until the frame is ready to continue
            LightEvent_Wait(&OnFrameResume);

            // Signal that the frame continue
            LightEvent_Pulse(&OnFrameResume);

            IsFramePaused = false;
            return;
        }

        bool drawRocket = Preferences::UseFloatingBtn && isBottom;
        bool drawTouch =  (Preferences::DrawTouchCursor || Preferences::DrawTouchCoord) && Touch::IsDown() && isBottom;
        bool drawFps = (Preferences::ShowBottomFps && isBottom) || (Preferences::ShowTopFps && !isBottom);

        if (!drawRocket && !drawTouch && !drawFps && !DrawSaveIcon && !MessColors
            && Callbacks.empty() && Notifications.empty())
            return;

        u32     size = isBottom ? stride * 320 : stride * 400;
        bool    mustFlush = drawFps;
        Handle  handle = Process::GetHandle();

        if (!isBottom)
        {
            ScreenImpl::Top->Acquire((u32)addr, (u32)addrB, stride, format & 0b111);
            Renderer::SetTarget(TOP);
        }
        else
        {
            ScreenImpl::Bottom->Acquire((u32)addr, (u32)addrB, stride, format & 0b111);
            Renderer::SetTarget(BOTTOM);
        }

        svcInvalidateProcessDataCache(handle, addr, size);

        if (!isBottom && addrB && addrB != addr)
            svcInvalidateProcessDataCache(handle, addrB, size);

        if (MessColors)
        {
            mustFlush = true;
            MessColor((u32)addr, stride, format);
        }

        // Draw notifications & icon
        if (!isBottom)
        {
            mustFlush |= OSDImpl::Draw() | DrawSaveIcon;

            if (DrawSaveIcon)
                Icon::DrawSave(10, 10);
        }
        // Draw touch cursor
        else
        {
            if (drawRocket)
            {
                FloatingBtn.Draw();
                mustFlush = true;
            }

            if (drawTouch)
            {
                IntVector touchPos(Touch::GetPosition());

                if (Preferences::DrawTouchCursor)
                {
                    int posX = touchPos.x - 2;
                    int posY = touchPos.y - 1;
                    Icon::DrawHandCursor(posX, posY);
                }
                if (Preferences::DrawTouchCoord)
                {
                    std::string &&str = Utils::Format("Touch.x: %d  Touch.y: %d", touchPos.x, touchPos.y);
                    int posY = 20;
                    Renderer::DrawString(str.c_str(), 10, posY, Color::Blank, Color::Black);
                }
                mustFlush = true;
            }
        }

        // Draw fps
        if (drawFps)
        {
            std::string &&fps = Utils::Format("FPS: %.02f", g_second / g_fpsClock[isBottom].Restart().AsSeconds());
            int posY = 10;
            Renderer::DrawString(fps.c_str(), 10, posY, Color::Blank, Color::Black);
            mustFlush |= true;
        }

        OSDImpl::Lock();
        // Call OSD Callbacks
        if (Callbacks.size())
        {
            Screen screen;

            screen.IsTop = !isBottom;
            screen.Is3DEnabled = isBottom ? false : ScreenImpl::Top->Is3DEnabled();
            screen.LeftFramebuffer = (u32)addr;
            screen.RightFramebuffer = (u32)addrB;
            screen.Stride = stride;
            screen.BytesPerPixel = GetBPP((GSPFormat)format);
            screen.Format = (GSPFormat)format;

            for (OSDCallback cb : Callbacks)
                mustFlush |= cb(screen);
        }
        OSDImpl::Unlock();

        if (mustFlush)
        {
            svcFlushProcessDataCache(handle, addr, size);

            if (!isBottom && addrB && addrB != addr)
                svcFlushProcessDataCache(handle, addrB, size);
        }
    }

    void    OSDImpl::UpdateScreens(void)
    {
        ScreenImpl *screen = ScreenImpl::Top;

        // Top screen
        TopScreen.IsTop = true;
        TopScreen.Is3DEnabled = screen->Is3DEnabled();
        TopScreen.LeftFramebuffer = (u32)screen->GetLeftFramebuffer(false);
        TopScreen.RightFramebuffer = (u32)screen->GetRightFramebuffer(false);
        TopScreen.Stride = (u32)screen->GetStride();
        TopScreen.BytesPerPixel = screen->GetBytesPerPixel();
        TopScreen.Format = screen->GetFormat();

        screen = ScreenImpl::Bottom;

        // Bottom screen
        BottomScreen.IsTop = false;
        BottomScreen.Is3DEnabled = screen->Is3DEnabled();
        BottomScreen.LeftFramebuffer = (u32)screen->GetLeftFramebuffer(false);
        BottomScreen.RightFramebuffer = (u32)screen->GetRightFramebuffer(false);
        BottomScreen.Stride = (u32)screen->GetStride();
        BottomScreen.BytesPerPixel = screen->GetBytesPerPixel();
        BottomScreen.Format = screen->GetFormat();
    }

    void    OSDImpl::WaitFramePaused(void)
    {
        if (IsFramePaused)
            return;

        LightEvent_Wait(&OnFramePaused);
    }

    void    OSDImpl::ResumeFrame(const u32 nbFrames)
    {
        if (!IsFramePaused)
            return;

        FramesToPlay = nbFrames;

        // Restore framebuffer if possible
        ScreenImpl::Clean();

        // Wake up game's thread
        LightEvent_Pulse(&OnFrameResume);

        if (nbFrames)
        {
            // Wait until all our frames are rendered and the process is paused again
            LightEvent_Wait(&OnFramePaused);
        }
    }

    static const u32    g_OSDPattern[] =
    {
        0xE1833000, // ORR R3, R3, R0
        0xE2044CFF, // AND R4, R4, #0xFF00
        0xE3C33CFF, // BIC R3, R3, #0xFF00
        0xE1833004, // ORR R3, R3, R4
        0xE1824F93, // STREX R4, R3, [R2] // 0x14

        0xE8830E60, // STMIA R3, {R5, R6, R9 - R11}
        0xEE078F9A, // MCR p15, 0, R8, c7, c10, 4 // Data Synchronization Barrier
        0xE3A03001, // MOV R3, #1
        0xE7902104, // LDR R2, [R0, R4, LSL#2] // 0x10

        0xEE076F9A, // MCR p15, 0, R6, c7, c10, 4 // Data Synchronization Barrier
        0xE3A02001, // MOV R2, #1
        0xE7901104, // LDR R1, [R0, R4, LSL#2]
        0xE1911F9F, // LDREX R1, [R1]
        0xE3C110FF, // BIC R1, R1, #0xFF // 0x14

        0xE3A00000, // MOV R0, #0
        0xEE070F9A, // MCR P15, 0, R0, c7, c10, 4
        0xE3A00001, // MOV R0, #1
        0xE7951104, // LDR R1, [R5, R4, LSL#2] // 0x10

        0xE3A00000, // MOV R0, #0
        0xEE070F9A, // MCR P15, 0, R0, c7, c10, 4
        0xE2850001, // ADD R0, R5, #1
        0xEA000004, // B #16 // 0x10
    };

    static u8       *memsearch(u8 *startPos, const void *pattern, u32 size, u32 patternSize)
    {
        const u8 *patternc = (const u8 *)pattern;
        u32 table[256];

        //Preprocessing
        for (u32 i = 0; i < 256; i++)
            table[i] = patternSize;
        for (u32 i = 0; i < patternSize - 1; i++)
            table[patternc[i]] = patternSize - i - 1;

        //Searching
        u32 j = 0;
        while (j <= size - patternSize)
        {
            u8 c = startPos[j + patternSize - 1];
            if (patternc[patternSize - 1] == c && memcmp(pattern, startPos + j, patternSize - 1) == 0)
                return startPos + j;
            j += table[c];
        }

        return nullptr;
    }

    static u32     SearchStmfd(u32 start, u32 size, u32 stmfd, bool &jump)
    {
        if (!start || !size || !stmfd)
            return (0);

        const u32   ntrhook = 0xE51FF004; // LDR PC, [PC, #-4]
        u32     result = 0;
        u32     *end = (u32 *)(start - size);

        for (u32 *addr = (u32 *)start; addr > end; addr--)
        {
            u32 val = *addr;

            if (val == stmfd)
            {
                result = (u32)addr;
                break;
            }

            if (val == ntrhook && (*(addr + 1) >> 24 == 0x06))
            {
                result = (u32)addr;
                jump = true;
                break;
            }
        }
        return (result);
    }

    static u32     SearchOSD(u32 pattern)
    {
        u8     *address = nullptr;

        if (pattern == 0)
        {
            address = memsearch((u8 *)0x100000, g_OSDPattern, Process::GetTextSize(), 0x14);

            if (address == nullptr)
            {
                address = memsearch((u8 *)0x100000, &g_OSDPattern[5], Process::GetTextSize(), 0x10);
            }
        }
        else if (pattern == 3)
        {
            address = memsearch((u8 *)0x100000, &g_OSDPattern[9], Process::GetTextSize(), 0x14);
        }
        else if (pattern == 4)
        {
            address = memsearch((u8 *)0x100000, &g_OSDPattern[14], Process::GetTextSize(), 0x10);
        }
        else if (pattern == 5)
        {
            address = memsearch((u8 *)0x100000, &g_OSDPattern[18], Process::GetTextSize(), 0x10);
        }

        return (u32)address;
    }

    void    InstallOSD(void)
    {
        static u32  returnCode[4];

        auto  createReturncode = [](u32 address, u32 *buf)
        {
            Process::CopyMemory(buf, (void *)address, 8);
            buf[2] = 0xE51FF004;
            buf[3] = address + 8;
        };

        const u32   stmfd2 = 0xE92D47F0; // STMFD SP!, {R4-R10,LR}
        const u32   stmfd1 = 0xE92D5FF0; // STMFD SP!, {R4-R12, LR}
        const u32   stmfd3 = 0xE92D4070; // STMFD SP!, {R4-R6, LR}

        bool        isHook = false;
        u32         found = SearchOSD(0);
        u32         found2 = 0;

        found = SearchStmfd(found, 0x400, stmfd1, isHook);
        if (!found)
        {
            found = SearchOSD(3);
            found = SearchStmfd(found, 0x400, stmfd2, isHook);
            if (!found)
            {
                found2 = SearchOSD(4);
                found2 = SearchStmfd(found2, 0x400, stmfd3, isHook);
                if (!found2)
                {
                    MessageBox("OSD couldn't be installed: #1 !")();
                    return;
                }
            }
        }

        OSDImpl::OSDHook.flags.useLinkRegisterToReturn = false;
        OSDImpl::OSDHook.flags.ExecuteOverwrittenInstructionBeforeCallback = false;

        if (found2)
        {
            createReturncode(found2, returnCode);
            OSDImpl::OSDHook.Initialize(found2, (u32)OSDImpl::MainCallback2);
            OSDImpl::OSDHook.Enable();
            if (!isHook)
                OSDImpl::HookReturn = (OSDReturn)returnCode;
            else
                OSDImpl::HookReturn = (OSDReturn)*((u32 *)found2 + 1);
            return;
        }

        createReturncode(found, returnCode);
        OSDImpl::OSDHook.Initialize(found, (u32)OSDImpl::MainCallback);
        OSDImpl::OSDHook.Enable();
        if (!isHook)
            OSDImpl::HookReturn = (OSDReturn)returnCode;
        else
            OSDImpl::HookReturn = (OSDReturn)*((u32 *)found + 1);
    }

    static void    MessColor(u32 startAddr, u32 stride, u32 format)
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

#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "font6x10Linux.h"

#include <vector>
#include <cstring>
#include "CTRPluginFramework/System/Sleep.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "Hook.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"
#include "CTRPluginFramework/System/Touch.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    bool    OSDImpl::DrawSaveIcon = false;
    bool    OSDImpl::MessColors = false;
    Hook    OSDImpl::OSDHook;
    RecursiveLock OSDImpl::RecLock;
    std::list<OSDImpl::OSDMessage*> OSDImpl::Notifications;
    std::vector<OSDCallback> OSDImpl::Callbacks;

    void    InstallOSD(void);

    void    OSDImpl::_Initialize(void)
    {
        InstallOSD();
        RecursiveLock_Init(&RecLock);
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
    int OSDImpl::MainCallback(u32 isBottom, int arg2, void* addr, void* addrB, int stride, int format, int arg7)
    {
        if (!addr)
            return (((OSDReturn)OSDHook.returnCode)(isBottom, arg2, addr, addrB, stride, format, arg7));

        if (ProcessImpl::_isPaused)
        {
            GSPGPU_SaveVramSysArea();
            svcSignalEvent(ProcessImpl::FrameEvent);
            RecursiveLock_Lock(&ProcessImpl::FrameLock);
            GSPGPU_RestoreVramSysArea();
            RecursiveLock_Unlock(&ProcessImpl::FrameLock);
        }

        bool drawTouch =  Preferences::DrawTouchCursor && Touch::IsDown() && isBottom;
        bool drawFps = (Preferences::ShowBottomFps && isBottom) || (Preferences::ShowTopFps && !isBottom);

        if (!drawTouch && !drawFps && !DrawSaveIcon && !MessColors
            && Callbacks.empty() && Notifications.empty())
            return (((OSDReturn)OSDHook.returnCode)(isBottom, arg2, addr, addrB, stride, format, arg7));

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
            if (drawTouch)
            {
                IntVector touchPos(Touch::GetPosition());

                int posX = touchPos.x - 2;
                int posY = touchPos.y - 1;
                Icon::DrawHandCursor(posX, posY);
                mustFlush = true;
            }
        }

        // Draw fps
        if (drawFps)
        {
            std::string &&fps = Utils::Format("FPS: %.02f", g_second / g_fpsClock[isBottom].Restart().AsSeconds());
            int posY = 10;
            Renderer::DrawString(fps.c_str(), 10, posY, Color::Blank, Color::Black);
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

        return (((OSDReturn)OSDHook.returnCode)(isBottom, arg2, addr, addrB, stride, format, arg7));

    }

    static const u32    g_OSDPattern[] =
    {
        0xE1833000, // ORR R3, R3, R0  ///< Here 0x14
        0xE2044CFF, // AND R4, R4, #0xFF00
        0xE3C33CFF, // BIC R3, R3, #0xFF00
        0xE1833004, // ORR R3, R3, R4
        0xE1824F93, // STREX R4, R3, [R2]
        0xE8830E60, // STMIA R3, {R5, R6, R9 - R11} ///< Here 0x10
        0xEE078F9A, // MCR p15, 0, R8, c7, c10, 4 // Data Synchronization Barrier
        0xE3A03001, // MOV R3, #1
        0xE7902104, // LDR R2, [R0, R4, LSL#2]
        0xEE076F9A, // MCR p15, 0, R6, c7, c10, 4 // Data Synchronization Barrier ///< Here 0x14
        0xE3A02001, // MOV R2, #1
        0xE7901104, // LDR R1, [R0, R4, LSL#2]
        0xE1911F9F, // LDREX R1, [R1]
        0xE3C110FF, // BIC R1, R1, #0xFF
        0x06200000  // STREQT R0, [R0], -R0
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

    static u32     SearchOSD(void)
    {
        u8  *address = memsearch((u8 *)0x100000, g_OSDPattern, Process::GetTextSize(), 0x14);

        if (address == nullptr)
        {
            address = memsearch((u8 *)0x100000, &g_OSDPattern[5], Process::GetTextSize(), 0x10);

            if (address == nullptr)
                address = memsearch((u8 *)0x100000, &g_OSDPattern[9], Process::GetTextSize(), 0x14);
        }

        return ((u32)address);
    }

    void    InstallOSD(void)
    {
        const u32   stmfd2 = 0xE92D47F0; // STMFD SP!, {R4-R10,LR}
        const u32   stmfd1 = 0xE92D5FF0; // STMFD SP!, {R4-R12, LR}
        u32         found = SearchOSD();
        u32         result = 0;
        u32         *end = (u32 *)(found - 0x400);

        if (!found)
        {
            MessageBox("OSD couldn't be installed: #1 !")();
            return;
        }

        // MessageBox(Utils::Format("OSD #1 Found: %08X", found))();

        for (u32 *addr = (u32 *)found; addr > end; addr--)
        {
            if (*addr == stmfd1)
            {
                result = (u32)addr;
                break;
            }
        }

        if (result == 0)
        {
            for (u32 *addr = (u32 *)found; addr > end; addr--)
            {
                if (*addr == stmfd2)
                {
                    result = (u32)addr;
                    break;
                }
            }

            if (result == 0)
            {
                MessageBox("OSD couldn't be installed: #2 !")();
                return;
            }
        }

        OSDImpl::OSDHook.Initialize(result, (u32)OSDImpl::MainCallback);
        OSDImpl::OSDHook.Enable();
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

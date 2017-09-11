#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "font6x10Linux.h"

#include <vector>
#include <cstring>
#include "NTR.hpp"
#include "NTRImpl.hpp"
#include "CTRPluginFramework/System/Sleep.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "Hook.hpp"
#include "CTRPluginFramework/System/Process.hpp"

namespace CTRPluginFramework
{
    OSDImpl     *OSDImpl::_single = nullptr;

	OSDImpl::OSDImpl(void) :
	_topModified(false),
	_bottomModified(false)
    {        
    }

    void    InstallOSD(void);
    void    OSDImpl::_Initialize(void)
    {
        InstallOSD();
        if (_single != nullptr)
            return;
        _single = new OSDImpl();
        RecursiveLock_Init(&_single->_lock);
    }

    OSDImpl     *OSDImpl::GetInstance(void)
    {
        return (_single);
    }

	void	OSDImpl::Start(void)
	{
       // Screen::Top->Acquire(true);
       // Screen::Top->Flush();
			
       // Screen::Bottom->Acquire(true);
      //  Screen::Bottom->Flush();
	}

	void	OSDImpl::Finalize(void)
	{
		if (_topModified)
		{
			Screen::Top->Invalidate();
			_topModified = false;
		}

		if (_bottomModified)
		{
			Screen::Bottom->Invalidate();
			_bottomModified = false;
		}
	}


#define XEND    390

    bool    OSDImpl::operator()(bool drawOnly)
    {
        Lock();

        if (_messages.empty())
        {
            Unlock();
            return (false);
        }

        if (!NTRImpl::IsOSDAvailable)
            Screen::Top->Acquire(true);
        else
        {
            PrivColor::SetFormat((GSPGPU_FramebufferFormats)NTRImpl::OSDParameters.format);
        }

        int posX;
        int posY = std::min((u32)15, (u32)_messages.size());
        posY = 230 - (15 * posY);

        if (drawOnly)
        {
            Unlock();
            return (true);
        }

        if (!NTR::IsOSDAvailable())
            Screen::Top->Invalidate();
        

        RecursiveLock_Unlock(&_lock);

        return (true);
    }

    void    OSDImpl::Update(void)
    {
        if (TryLock())
            return;

        while (_messages.size() && _messages.front()->drawn)
        {
            OSDMessage *message = _messages.front();

            if (message->time.HasTimePassed(Seconds(5.f)))
            {
                delete message;
                _messages.pop_front();
            }
            else
                break;
        }

        Unlock();
    }

    bool    OSDImpl::Draw(void)
    {
        Lock();

        if (_messages.empty())
        {
            Unlock();
            return (false);
        }

        if (!NTRImpl::IsOSDAvailable)
            Screen::Top->Acquire(true);
        else
        {
            PrivColor::SetFormat((GSPGPU_FramebufferFormats)NTRImpl::OSDParameters.format);
        }

        int posX;
        int posY = std::min((u32)15, (u32)_messages.size());
        posY = 230 - (15 * posY);
        int count = 0;

        for (OSDMessage *message : _messages)
        {
            posX = XEND - message->width;
            _DrawMessage(*message, posX, posY);

            if (!message->drawn)
                message->time.Restart();

            message->drawn = true;

            count++;
            if (count >= 15)
                break;
        }

        if (!NTR::IsOSDAvailable())
            Screen::Top->Invalidate();

        Unlock();
        return (true);
    }

    void    OSDImpl::Lock(void)
    {
        RecursiveLock_Lock(&_lock);
    }

    bool OSDImpl::TryLock(void)
    {
        return (RecursiveLock_TryLock(&_lock));
    }

    void    OSDImpl::Unlock(void)
    {
        RecursiveLock_Unlock(&_lock);
    }

    void    OSDImpl::_DrawMessage(OSDMessage &message, int posX, int &posY)
    {
        _DrawTop(message.text, posX, posY, 10, message.foreground, message.background);
    }

    void    OSDImpl::_DrawTop(std::string &text, int posX, int &posY, int offset, Color &fg, Color &bg)
    {
       // Screen::Top->Acquire(true);

        const char  *str = text.c_str();
        int         stride = NTRImpl::IsOSDAvailable ? NTRImpl::OSDParameters.stride : Screen::Top->GetStride();
        int         off3D = offset < 0 ? offset - 1 : offset + 1;

       // _topModified = true;

        if (NTRImpl::IsOSDAvailable)
        {
            if (NTRImpl::OSDParameters.is3DEnabled)
            {
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
                while (*str)
                {
                    char c = *str;
                    int index = c * 10;

                    for (int yy = 0; yy < 10; yy++)
                    {
                        u8 charPos = font[index + yy];

                        int x = 0;
                        u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX, posY + yy);
                        u8  *framebuf1 = (u8 *)NTR::GetRightFramebuffer(posX - offset, posY + yy);
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
                    str++;
                    posX += 6;
                }            
            }
            // No 3D
            else
            {
                {
                    for (int x = -2; x < 0; x++)
                    {
                        for (int i = 0; i < 10; i++)
                        {
                            u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX + x, posY + i);
                            PrivColor::ToFramebuffer(framebuf0, bg);
                        }
                    }

                }
                while (*str)
                {
                    char c = *str;
                    int index = c * 10;

                    for (int yy = 0; yy < 10; yy++)
                    {
                        u8 charPos = font[index + yy];

                        int x = 0;
                        u8  *framebuf0 = (u8 *)NTR::GetLeftFramebuffer(posX, posY + yy);
                        for (int xx = 6; xx >= 0; xx--, x++)
                        {
                            if ((charPos >> xx) & 1)
                            {
                                PrivColor::ToFramebuffer(framebuf0, fg);
                            }
                            else
                            {
                                PrivColor::ToFramebuffer(framebuf0, bg);
                            }
                            framebuf0 += stride;
                        }
                    }
                    str++;
                    posX += 6;
                }
            }
            
            posY += 15;
            return;
        }

        // If 3D
        if (*(float *)(0x1FF81080) > 0.f)
        {
            {
                for (int x = -2; x < 0; x++)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX + x, posY + i);
                        u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX + x, posY + i, true);
                        u8  *framebuf2 = Screen::Top->GetRightFramebuffer(posX - off3D, posY + i);
                        u8  *framebuf3 = Screen::Top->GetRightFramebuffer(posX - off3D, posY + i, true);
                        PrivColor::ToFramebuffer(framebuf0, bg);
                        PrivColor::ToFramebuffer(framebuf1, bg);
                        PrivColor::ToFramebuffer(framebuf2, bg);
                        PrivColor::ToFramebuffer(framebuf3, bg);
                    }
                }
            }
            while (*str)
            {
                char c = *str;

                for (int yy = 0; yy < 10; yy++)
                {
                    u8 charPos = font[c * 10 + yy];
                    u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX, posY + yy);
                    u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX, posY + yy, true);
                    u8  *framebuf2 = Screen::Top->GetRightFramebuffer(posX - offset, posY + yy);
                    u8  *framebuf3 = Screen::Top->GetRightFramebuffer(posX - offset, posY + yy, true);

                    int x = 0;
                    for (int xx = 6; xx >= 0; xx--, x++)
                    {
                        if ((charPos >> xx) & 1)
                        {
                            PrivColor::ToFramebuffer(framebuf0, fg);
                            PrivColor::ToFramebuffer(framebuf1, fg);
                            PrivColor::ToFramebuffer(framebuf2, fg);
                            PrivColor::ToFramebuffer(framebuf3, fg);
                        }
                        else
                        {
                            PrivColor::ToFramebuffer(framebuf0, bg);
                            PrivColor::ToFramebuffer(framebuf1, bg);
                            PrivColor::ToFramebuffer(framebuf2, bg);
                            PrivColor::ToFramebuffer(framebuf3, bg);
                        }
                        framebuf0 += stride;
                        framebuf1 += stride;
                        framebuf2 += stride;
                        framebuf3 += stride;
                    }
                }
                str++;
                posX += 6;
            }
        }
        else
        {
            {
                for (int x = -2; x < 0; x++)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX + x, posY + i);
                        u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX + x, posY + i, true);
                        PrivColor::ToFramebuffer(framebuf0, bg);
                        PrivColor::ToFramebuffer(framebuf1, bg);
                    }
                }

            }
            while (*str)
            {
                char c = *str;
                int index = c * 10;

                for (int yy = 0; yy < 10; yy++)
                {
                    u8 charPos = font[index + yy];

                    int x = 0;
                    u8  *framebuf0 = Screen::Top->GetLeftFramebuffer(posX, posY + yy);
                    u8  *framebuf1 = Screen::Top->GetLeftFramebuffer(posX, posY + yy, true);
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
                str++;
                posX += 6;
            }
        }
        posY += 15;
    }

    void    OSDImpl::_DrawBottom(std::string &text, int posX, int &posY, Color &fg, Color &bg)
    {
        Screen::Bottom->Acquire(true);

        const char *str = text.c_str();
        int     stride = Screen::Bottom->GetStride();

        _bottomModified = true;

        {
            for (int x = -2; x < 0; x++)
            {
                for (int i = 0; i < 10; i++)
                {
                    u8  *framebuf0 = Screen::Bottom->GetLeftFramebuffer(posX + x, posY + i);
                    u8  *framebuf1 = Screen::Bottom->GetLeftFramebuffer(posX + x, posY + i, true);
                    PrivColor::ToFramebuffer(framebuf0, bg);
                    PrivColor::ToFramebuffer(framebuf1, bg);
                }
            }

        }
        while (*str)
        {
            char c = *str;
            int index = c * 10;

            for (int yy = 0; yy < 10; yy++)
            {
                u8 charPos = font[index + yy];

                int x = 0;
                u8  *framebuf0 = Screen::Bottom->GetLeftFramebuffer(posX, posY + yy);
                u8  *framebuf1 = Screen::Bottom->GetLeftFramebuffer(posX, posY + yy, true);
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
            str++;
            posX += 6;
        }
        posY += 15;

        Screen::Bottom->Invalidate();
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

    u32     SearchOSD(void)
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

    u32    MainOverlayCallback(u32 isBottom, u32 addr, u32 addrB, u32 stride, u32 format);

    static Hook g_osdHook;
    using OSDReturn = int(*)(u32, int, void *, void *, int, int, int);

    int OSDHooked(u32 isBottom, int arg2, void *addr, void *addrB, int stride, int format, int arg7)
    {
        if (!addr)
            return (((OSDReturn)g_osdHook.returnCode)(isBottom, arg2, addr, addrB, stride, format, arg7));

        u32     size = isBottom ? stride * 320 : stride * 400;
        Handle  handle = Process::GetHandle();

        svcInvalidateProcessDataCache(handle, addr, size);

        if (!isBottom && addrB && addrB != addr)
            svcInvalidateProcessDataCache(handle, addrB, size);

        if (MainOverlayCallback(isBottom, (u32)addr, (u32)addrB, stride, format))
        {
            svcFlushProcessDataCache(handle, addr, size);

            if (!isBottom && addrB && addrB != addr)
                svcInvalidateProcessDataCache(handle, addrB, size);
        }

        return (((OSDReturn)g_osdHook.returnCode)(isBottom, arg2, addr, addrB, stride, format, arg7));
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

        //MessageBox(Utils::Format("OSD #2 Found: %08X", result))();

        g_osdHook.Initialize(result, (u32)OSDHooked);
        g_osdHook.Enable();
    }
}

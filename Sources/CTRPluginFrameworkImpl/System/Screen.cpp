#include "types.h"

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFramework/System/System.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "ctrulib/allocator/vram.h"
#include "ctrulib/allocator/mappable.h"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "csvc.h"
#include "ctrulib/ipc.h"
#include "ctrulib/srv.h"

namespace CTRPluginFramework
{
    #define REG(x)   *(vu32 *)(x)
    #define REG32(x) *(vu32 *)((x) | (1u << 31))

    namespace GSP
    {
        enum
        {
            FB_TOP_READY = BIT(0),
            FB_BOTTOM_READY = BIT(1),

            FB_BOTH_READY = (FB_TOP_READY | FB_BOTTOM_READY),

            FB_TOP_NEED_CLEAR = BIT(2),
            FB_BOTTOM_NEED_CLEAR = BIT(3),

            FB_BOTH_NEED_CLEAR = (FB_TOP_NEED_CLEAR | FB_BOTTOM_NEED_CLEAR)
        };

        static void   InterruptReceiver(void *arg);

        static bool                     RunInterruptReceiver{true};
        static bool                     CatchInterrupt{false};
        static u32                      ThreadId{0};
        static s32                      BufferFlags{0};
        static vu8 *                    SharedMemoryBlock{nullptr};
        static vu8 *                    EventData{nullptr};
        static volatile Handle          GSPEvent;
        static Handle                   WakeEvent;
        static LightEvent               VBlank0Event;
        static LightEvent               VBlank1Event;
        static Hook                     GSPRegisterInterruptReceiverHook;
        static ThreadEx                 InterruptReceiverThread{InterruptReceiver, 0x1000, 0x35, -1};
        static FrameBufferInfoShared *  SharedFrameBuffers[2]{nullptr};
        u32    InterruptReceiverThreadPriority;

        extern "C" void GSPGPU__RegisterInterruptHook(void);
        extern "C" void __gsp__Update(u32 threadId, Handle eventHandle, Handle sharedMemHandle)
        {
            Update(threadId, eventHandle, sharedMemHandle);
        }

        Result  Initialize(void)
        {
            const std::vector<u32> gspgpuRegisterInterruptPattern =
            {
                0xE92D4070,
                0xE1A06003,
                0xE59D5010,
                0xEE1D3F70,
                0xE2834080,
                0xE59F3034,
                0xE3A0C000,
                0xE584100C,
                0xE9841004,
                0xE5843000,
                0xE5900000,
                0xEF000032,
                0xE2101102,
                0x4A000004,
                0xE5940008,
                0xE5850000,
                0xE5940010,
                0xE5860000,
                0xE5940004,
                0xE8BD8070,
                0x00130042,
            };

            const std::vector<u32> gspgpuRegisterInterruptPattern2 =
            {
                0xE92D4070,
                0xE1A05003,
                0xE59D6010,
                0xEE1D4F70,
                0xE59FC034,
                0xE3A03000,
                0xE5A4C080,
                0xE584100C,
                0xE1C420F4,
                0xE5900000,
                0xEF000032,
                0xE2101102,
                0x4A000004,
                0xE5940008,
                0xE5860000,
                0xE5940010,
                0xE5850000,
                0xE5940004,
                0xE8BD8070,
                0x00130042
            };


            u32 addr = Utils::Search(0x00100000, Process::GetTextSize(), gspgpuRegisterInterruptPattern);
            Hook& hook = GSPRegisterInterruptReceiverHook;

            if (addr)
            {
                hook.Initialize(addr + 0x2C, (u32)GSPGPU__RegisterInterruptHook);
                hook.flags.ExecuteOverwrittenInstructionBeforeCallback = false;
                hook.Enable();
            }
            else
            {
                addr = Utils::Search(0x00100000, Process::GetTextSize(), gspgpuRegisterInterruptPattern2);
                if (addr)
                {
                    hook.Initialize(addr + 0x28, (u32)GSPGPU__RegisterInterruptHook);
                    hook.flags.ExecuteOverwrittenInstructionBeforeCallback = false;
                    hook.Enable();
                }
                else
                    return -1;
            }

            svcCreateEvent(&WakeEvent, RESET_ONESHOT);
            LightEvent_Init(&VBlank0Event, RESET_STICKY);
            LightEvent_Init(&VBlank1Event, RESET_STICKY);

            return 0;
        }

        void    Update(u32 threadId, Handle eventHandle, Handle sharedMemHandle)
        {
            ThreadId = threadId;
            GSPEvent = eventHandle;

            if (SharedMemoryBlock == nullptr)
            {
                SharedMemoryBlock = static_cast<vu8 *>(mappableAlloc(0x1000));
                svcMapMemoryBlock(sharedMemHandle, reinterpret_cast<u32>(SharedMemoryBlock), static_cast<MemPerm>(0x3), static_cast<MemPerm>(0x10000000));
            }

            u8  *base = const_cast<u8 *>(SharedMemoryBlock) + 0x200 + threadId * 0x80;
            SharedFrameBuffers[0] = reinterpret_cast<FrameBufferInfoShared *>(base);
            SharedFrameBuffers[1] = reinterpret_cast<FrameBufferInfoShared *>(base + 0x40);
            EventData = SharedMemoryBlock + threadId * 0x40;

            if (InterruptReceiverThread.GetStatus() == ThreadEx::IDLE)
            {
                RunInterruptReceiver = true;
                CatchInterrupt = false;
                InterruptReceiverThread.priority = InterruptReceiverThreadPriority;
                InterruptReceiverThread.Start(nullptr);
            }
        }

        void    FrameBufferInfo::FillFrameBufferFrom(FrameBufferInfo &src)
        {
            // Only support this.format == RGB_565
            if ((format & 0xF) != GSP_RGB565_OES)
                return;

            union
            {
                u16     u;
                char    b[2];
            }           half;

            u8  *pix = reinterpret_cast<u8 *>(src.framebuf0_vaddr);
            u8  *dst = reinterpret_cast<u8 *>(framebuf0_vaddr);
            u32  width = (src.format & 0x60) > 0 ? 400 : 320;

            svcInvalidateProcessDataCache(CUR_PROCESS_HANDLE, src.framebuf0_vaddr, width * src.framebuf_widthbytesize);

            switch (src.format & 7)
            {
            case GSP_RGBA8_OES:
            {
                u32 padding = src.framebuf_widthbytesize - 4 * 240;

                for (; width > 0; --width)
                {
                    for (u32 height = 240; height > 0; --height)
                    {
                        half.u  = (*pix++ & 0xF8) << 8;
                        half.u |= (*pix++ & 0xFC) << 3;
                        half.u |= (*pix++ & 0xF8) >> 3;

                        *dst++ = half.b[0];
                        *dst++ = half.b[1];
                        ++pix;
                    }
                    pix += padding;
                }

                break;
            }
            case GSP_BGR8_OES:
            {
                u32 padding = src.framebuf_widthbytesize - 3 * 240;

                for (; width > 0; --width)
                {
                    for (u32 height = 240; height > 0; --height)
                    {
                        half.u = (*pix++ & 0xF8) >> 3;
                        half.u |= (*pix++ & 0xFC) << 3;
                        half.u |= (*pix++ & 0xF8) << 8;

                        *dst++ = half.b[0];
                        *dst++ = half.b[1];
                    }
                    pix += padding;
                }

                break;
            }
            case GSP_RGB565_OES:
            {
                u32 size = 240 * 2;

                for (; width > 0; --width)
                {
                    std::copy(pix, pix + size, dst);
                    pix += src.framebuf_widthbytesize;
                    dst += size;
                }
                break;
            }
            case GSP_RGB5_A1_OES:
            {
                u32 padding = src.framebuf_widthbytesize - 2 * 240;

                union
                {
                    u16     u{0};
                    u8      b[2];
                }           col;

                for (; width > 0; --width)
                {
                    for (u32 height = 240; height > 0; --height)
                    {
                        col.b[0] = *pix++;
                        col.b[1] = *pix++;

                        half.u  = ((col.u >> 8) & 0xF8) << 8;
                        half.u |= ((col.u >> 3) & 0xF8) << 3;
                        half.u |= ((col.u << 2) & 0xF8) >> 3;

                        *dst++ = half.b[0];
                        *dst++ = half.b[1];
                    }
                    pix += padding;
                }

                break;
            }
            case GSP_RGBA4_OES:
            {
                u32 padding = src.framebuf_widthbytesize - 2 * 240;

                union
                {
                    u16     u{0};
                    u8      b[2];
                }           col;

                for (; width > 0; --width)
                {
                    for (u32 height = 240; height > 0; --height)
                    {
                        col.b[0] = *pix++;
                        col.b[1] = *pix++;

                        half.u  = ((col.u >> 8) & 0xF0) << 8;
                        half.u |= ((col.u >> 4) & 0xF0) << 3;
                        half.u |= (col.u & 0xF0) >> 3;

                        *dst++ = half.b[0];
                        *dst++ = half.b[1];
                    }
                    pix += padding;
                }

                break;
            }
            default:
                break;
            }
        }

        void    FrameBufferInfoShared::FillFrameBuffersFrom(FrameBufferInfoShared &src)
        {
            const u32 displayed = src.header.screen;
            //const u32 size = 240 * 2 * (src.fbInfo[displayed].framebuf1_vaddr == nullptr ? 320 : 400);

            fbInfo[0].FillFrameBufferFrom(src.fbInfo[displayed]);
            //std::copy(fbInfo[0].framebuf0_vaddr, fbInfo[0].framebuf0_vaddr + (size >> 2), \
                      fbInfo[1].framebuf0_vaddr);
            //svcFlushProcessDataCache(CUR_PROCESS_HANDLE, fbInfo[1].framebuf0_vaddr, size);
        }

        static void  ClearInterrupts(void)
        {
	        bool    strexFailed;

	        do
            {
		        // Do a load on all header fields as an atomic unit
		        __ldrex((s32 *)EventData);

		        strexFailed = __strex((s32 *)EventData, 0);

	        } while (__builtin_expect(strexFailed, 0));
        }

        static int  PopInterrupt(void)
        {
	        int     curEvt;
	        bool    strexFailed;

	        do
            {
		        union
	            {
                    u32     as_u32;
			        struct
		            {
				        u8  cur;
				        u8  count;
				        u8  err;
				        u8  unused;
			        };
                } header;

		        // Do a load on all header fields as an atomic unit
		        header.as_u32 = __ldrex((s32 *)EventData);

		        if (__builtin_expect(header.count == 0, 0))
                {
			        __clrex();
			        return -1;
		        }

		        curEvt = EventData[0xC + header.cur];

		        header.cur += 1;
		        if (header.cur >= 0x34)
                    header.cur -= 0x34;
		        header.count -= 1;
		        header.err = 0; // Should this really be set?

		        strexFailed = __strex((s32 *)EventData, header.as_u32);

	        } while (__builtin_expect(strexFailed, 0));

	        return curEvt;
        }

        static s32      __ldrex__(s32 *addr)
        {
            s32 v;
            do
                v = __ldrex(addr);
            while (__strex(addr, v));

            return v;
        }

        static void     __strex__(s32 *addr, s32 val)
        {
            do
                __ldrex(addr);
            while (__strex(addr, val));
        }

        static void InterruptReceiver(void *arg UNUSED)
        {
            while (RunInterruptReceiver)
            {
                while (!CatchInterrupt)
                {
                    if (!RunInterruptReceiver)
                        break;
                    svcWaitSynchronization(WakeEvent, U64_MAX);
                }

                ClearInterrupts();
                while (CatchInterrupt)
                {
                    svcClearEvent(GSPEvent);
                    svcWaitSynchronization(GSPEvent, U64_MAX);

                    while (true)
                    {
			            int curEvt = PopInterrupt();

			            if (curEvt == -1)
				            break;

                        // Top screen event
			            if (curEvt == GSPGPU_EVENT_VBlank0)
			            {
				            LightEvent_Signal(&VBlank0Event);
			            }

                        // Bottom screen event
			            if (curEvt == GSPGPU_EVENT_VBlank1)
			            {
                            LightEvent_Signal(&VBlank1Event);
			            }
                    }
                }
            }

            svcExitThread();
        }

        void    PauseInterruptReceiver(void)
        {
            svcClearEvent(WakeEvent);
            CatchInterrupt = false;
        }

        void    ResumeInterruptReceiver(void)
        {
            CatchInterrupt = true;
            svcSignalEvent(WakeEvent);
        }

        void    WaitForVBlank(void)
        {
            ClearInterrupts();
            LightEvent_Clear(&VBlank0Event);
            LightEvent_Wait(&VBlank0Event);
        }

        void    WaitForVBlank1(void)
        {
            ClearInterrupts();
            LightEvent_Clear(&VBlank1Event);
            LightEvent_Wait(&VBlank1Event);
        }

        void    SwapBuffer(int screen)
        {
            s32 *addr = &SharedFrameBuffers[screen]->header.header;
            s32 val;
            do
            {
                val = __ldrex(addr);
                val ^= 1;       // Toggle displayed screen
                val |= 0x100;   // Update flag
            } while (__strex(addr, val));
        }

        void    WaitBufferSwapped(int screen)
        {
            if (screen == 1)
            {
                WaitForVBlank1();
                goto __clearBottom;
            }

            WaitForVBlank();

            if (BufferFlags & FB_TOP_NEED_CLEAR)
            {
                BufferFlags &= ~FB_TOP_NEED_CLEAR;
                ScreenImpl::Top->Clear(false);
            }

            if (screen == 0)
                return;

        __clearBottom:
            if (BufferFlags & FB_BOTTOM_NEED_CLEAR)
            {
                BufferFlags &= ~FB_BOTTOM_NEED_CLEAR;
                ScreenImpl::Bottom->Clear(false);
            }
        }

        void    ImportFrameBufferInfo(FrameBufferInfoShared& dest, int screen)
        {
            u8 *src = reinterpret_cast<u8 *>(SharedFrameBuffers[screen]);
            u8 *dst = reinterpret_cast<u8 *>(&dest);

            std::copy(src, src + sizeof(FrameBufferInfoShared), dst);
        }

        static u32 *plgVAtoGameVa(u32 *va)
        {
            return reinterpret_cast<u32 *>(svcConvertVAToPA(va, false) - 0xC000000);
        }

        void    SetFrameBufferInfo(FrameBufferInfoShared& src, int screen, bool convert)
        {
            u8 *s = reinterpret_cast<u8 *>(&src.fbInfo);
            u8 *dst = reinterpret_cast<u8 *>(&SharedFrameBuffers[screen]->fbInfo);

            std::copy(s, s + sizeof(FrameBufferInfo) * 2, dst);

            // VA to PA to expected VA
            if (convert)
            {
                FrameBufferInfoShared *fbs = SharedFrameBuffers[screen];

                fbs->fbInfo[0].framebuf0_vaddr = plgVAtoGameVa(src.fbInfo[0].framebuf0_vaddr);
                if (src.fbInfo[0].framebuf1_vaddr)
                    fbs->fbInfo[0].framebuf1_vaddr = plgVAtoGameVa(src.fbInfo[0].framebuf1_vaddr);

                fbs->fbInfo[1].framebuf0_vaddr = plgVAtoGameVa(src.fbInfo[1].framebuf0_vaddr);
                if (src.fbInfo[1].framebuf1_vaddr)
                    fbs->fbInfo[1].framebuf1_vaddr = plgVAtoGameVa(src.fbInfo[0].framebuf1_vaddr);
            }

            src.header.update = 1;
            s32 *addr = &SharedFrameBuffers[screen]->header.header;
            do
            {
                __ldrex(addr);
            } while (__strex(addr, src.header.header));

            if (screen) WaitForVBlank();
            else WaitForVBlank1();
        }
    } ///< GSP

    struct ScreensFramebuffers
    {
        u8  topFramebuffer0[400 * 240 * 2];
        u8  topFramebuffer1[400 * 240 * 2];
        u8  bottomFramebuffer0[320 * 240 * 2];
        u8  bottomFramebuffer1[320 * 240 * 2];
    } PACKED;

    // Reserve the place for the Screen objects
    static u8  _topBuf[sizeof(ScreenImpl)] ALIGN(4);
    static u8  _botBuf[sizeof(ScreenImpl)] ALIGN(4);

    ScreenImpl  *ScreenImpl::Top = nullptr;
    ScreenImpl  *ScreenImpl::Bottom = nullptr;

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

    ScreenImpl::ScreenImpl(const u32 lcdSetupInfo, const u32 fillColorAddress, const bool isTopScreen) :
        _LCDSetup(lcdSetupInfo),
        _FillColor(fillColorAddress),
        _backlightOffset{isTopScreen ? 0x40u : 0x840u},
        _currentBuffer(0),
        _width(isTopScreen ? 400 : 320),
        _stride(0), _rowSize(0), _bytesPerPixel(0),
        _isTopScreen(isTopScreen), _format()
    {
    }

    void    ScreenImpl::Initialize(void)
    {
        auto *screensFbs = reinterpret_cast<ScreensFramebuffers *>(FwkSettings::Header->heapVA);

        Top = new (_topBuf) ScreenImpl(0x10400400 | (1u << 31), (0x10202000 | (1u << 31)) + 0x204, true);
        Bottom = new (_botBuf) ScreenImpl(0x10400500 | (1u << 31), (0x10202000 | (1u << 31)) + 0xA04);

        // Top screen
        GSP::FrameBufferInfo* fb = &Top->_frameBufferInfo.fbInfo[0];

        fb->active_framebuf = 0;
        fb->framebuf0_vaddr = reinterpret_cast<u32 *>(screensFbs->topFramebuffer0);
        fb->framebuf1_vaddr = fb->framebuf0_vaddr;
        fb->framebuf_widthbytesize = 240 * 2; // Enforce RGB565
        fb->format = 0x42; // GSP_RGB565_OES;
        fb->framebuf_dispselect = 0;

        fb = &Top->_frameBufferInfo.fbInfo[1];

        fb->active_framebuf = 1;
        fb->framebuf0_vaddr = reinterpret_cast<u32 *>(screensFbs->topFramebuffer1);
        fb->framebuf1_vaddr = fb->framebuf0_vaddr;
        fb->framebuf_widthbytesize = 240 * 2; // Enforce RGB565
        fb->format = 0x42; // GSP_RGB565_OES;
        fb->framebuf_dispselect = 1;

        // Bottom screen
        fb = &Bottom->_frameBufferInfo.fbInfo[0];

        fb->active_framebuf = 0;
        fb->framebuf0_vaddr = reinterpret_cast<u32 *>(screensFbs->bottomFramebuffer0);
        fb->framebuf1_vaddr = nullptr;
        fb->framebuf_widthbytesize = 240 * 2; // Enforce RGB565
        fb->format = GSP_RGB565_OES;
        fb->framebuf_dispselect = 0;

        fb = &Bottom->_frameBufferInfo.fbInfo[1];

        fb->active_framebuf = 1;
        fb->framebuf0_vaddr = reinterpret_cast<u32 *>(screensFbs->bottomFramebuffer1);
        fb->framebuf1_vaddr = nullptr;
        fb->framebuf_widthbytesize = 240 * 2; // Enforce RGB565
        fb->format = GSP_RGB565_OES;
        fb->framebuf_dispselect = 1;
    }

    void    ScreenImpl::Fade(const float fade)
    {
        const u32   size = GetFrameBufferSize() / _bytesPerPixel;
        u8         *frameBuf = GetLeftFrameBuffer();

        PrivColor::SetFormat(_format);

        for (int i = size; i > 0; --i)
        {
            frameBuf = PrivColor::ToFramebuffer(frameBuf, PrivColor::FromFramebuffer(frameBuf).Fade(fade));
        }
    }

    u32     ScreenImpl::Acquire(void)
    {
        // Fetch game frame buffers & check validity
        if (ImportFromGsp())
            return -1;

        // Copy images and convert to RGB565 to ctrpf's frame buffer (0)
        _frameBufferInfo.FillFrameBuffersFrom(_gameFrameBufferInfo);

        _currentBuffer = _frameBufferInfo.header.screen = 1;

        _format = GSP_RGB565_OES;
        _stride = 240*2;
        _bytesPerPixel = 2;
        _rowSize = 240;

        auto &fbInfo = _frameBufferInfo.fbInfo;

        _leftFrameBuffers[0] = reinterpret_cast<u32>(fbInfo[0].framebuf0_vaddr);
        _leftFrameBuffers[1] = reinterpret_cast<u32>(fbInfo[1].framebuf0_vaddr);
        _rightFrameBuffers[0] = reinterpret_cast<u32>(fbInfo[0].framebuf1_vaddr);
        _rightFrameBuffers[1] = reinterpret_cast<u32>(fbInfo[1].framebuf1_vaddr);

        // Apply fade to fb0
        Fade(0.3f);

        // Copy to fb1
        _currentBuffer = 0;
        Copy();
        Flush();
        _currentBuffer = 1;

        // Apply current frame buffers
        GSP::SetFrameBufferInfo(_frameBufferInfo, !_isTopScreen, true);

        return 0;
    }

    u32     ScreenImpl::ImportFromGsp(void)
    {
        // Fetch game frame buffers
        GSP::ImportFrameBufferInfo(_gameFrameBufferInfo, !_isTopScreen);

        // Check frame buffers validity
        const u32 displayed = _gameFrameBufferInfo.header.screen;
        if (!Process::CheckAddress(reinterpret_cast<u32>(_gameFrameBufferInfo.fbInfo[displayed].framebuf0_vaddr)))
            return -1;

        return 0;
    }

    void    ScreenImpl::Release(void)
    {
        GSP::SetFrameBufferInfo(_gameFrameBufferInfo, !_isTopScreen, false);
    }

    void    ScreenImpl::Acquire(u32 left, u32 right, u32 stride, u32 format, bool backup)
    {
        _currentBuffer = 1;

        _format = static_cast<GSPGPU_FramebufferFormats>(format & 7);
        _stride = stride;
        _bytesPerPixel = GetBPP(_format);
        _rowSize = _stride / _bytesPerPixel;
        _leftFrameBuffers[0] = left;
        _rightFrameBuffers[0] = right;
    }

    void    ScreenImpl::Flush(void)
    {
        u32 size = GetFrameBufferSize();

        // Flush currentBuffer
        svcFlushProcessDataCache(CUR_PROCESS_HANDLE, GetLeftFrameBuffer(), size);
    }

	void	ScreenImpl::Invalidate(void)
	{
		u32 size = GetFrameBufferSize();

		// Invalidate currentBuffer
		svcInvalidateProcessDataCache(CUR_PROCESS_HANDLE, GetLeftFrameBuffer(), size);
	}

    void    ScreenImpl::Clear(bool applyFlagForCurrent)
    {
        const u32 displayed = _gameFrameBufferInfo.header.screen;

        _frameBufferInfo.fbInfo[!_currentBuffer].FillFrameBufferFrom(_gameFrameBufferInfo.fbInfo[displayed]);

        Fade(0.3f);
        if (!applyFlagForCurrent)
            return;

        s32 val = GSP::__ldrex__(&GSP::BufferFlags);

        val |= (_isTopScreen ? GSP::FB_TOP_NEED_CLEAR : GSP::FB_BOTTOM_NEED_CLEAR);
        GSP::__strex__(&GSP::BufferFlags, val);
    }

    void    ScreenImpl::Copy(void)
    {
        u32 size = GetFrameBufferSize();
        u8 *dst = GetLeftFrameBuffer();
        u8 *src = GetLeftFrameBuffer(true);

        // Copy the framebuffer to the second framebuffer (avoid the sensation of flickering on buffer swap)
        std::copy(src, src + size, dst);
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
        return _isTopScreen;
    }

    using Pixel = BMPImage::Pixel;

    static void    ScreenToBMP_BGR8(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride)
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

    static void    ScreenToBMP_RGBA8(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride)
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

    static void    ScreenToBMP_RGB565(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride)
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

    static void    ScreenToBMP_RGB5A1(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride)
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

    static void    ScreenToBMP_RGBA4(Pixel *bmp, u32 padding, u8 *src, u32 width, u32 stride)
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

    void    ScreenImpl::ScreenToBMP(Pixel *bmp, const u32 padding)
    {
        if (bmp == nullptr)
            return;

        u8      *src = nullptr;

        if (src == nullptr)
            src = GetLeftFrameBuffer();

        if (_format == GSP_RGBA8_OES) return ScreenToBMP_RGBA8(bmp, padding, src, _width, _stride);
        if (_format == GSP_BGR8_OES) return ScreenToBMP_BGR8(bmp, padding, src, _width, _stride);
        if (_format == GSP_RGB565_OES) return ScreenToBMP_RGB565(bmp, padding, src, _width, _stride);
        if (_format == GSP_RGB5_A1_OES) return ScreenToBMP_RGB5A1(bmp, padding, src, _width, _stride);
        if (_format == GSP_RGBA4_OES) return ScreenToBMP_RGBA4(bmp, padding, src, _width, _stride);
    }

    static inline BMPImage* CreateBMP(u32 width, u32 height)
    {
        BMPImage *image = new BMPImage(width, height);

        if (image->data() == nullptr && !SystemImpl::IsNew3DS)
        {
            Preferences::UnloadBackgrounds();
            delete image;
            image = new BMPImage(width, height);
            if (image->data() == nullptr)
                OSD::Notify("An error occured when trying to allocate the BMP");
        }

        return image;
    }

    BMPImage *ScreenImpl::ScreenShot(int screen, BMPImage *image)
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
            return false;

        u32 left = _leftFrameBuffers[!_currentBuffer];
        u32 right = _rightFrameBuffers[!_currentBuffer];

        return right && right != left && *(float *)(0x1FF81080) > 0.f;
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

        Top->Clear(true);
        Bottom->Clear(true);
    }

    void    ScreenImpl::SwitchFrameBuffers(bool game)
    {
        if (game)
        {
            Top->Release();
            Bottom->Release();
        }
        else
        {
            Top->_frameBufferInfo.header.screen = Top->_currentBuffer;
            Bottom->_frameBufferInfo.header.screen = Bottom->_currentBuffer;
            GSP::SetFrameBufferInfo(Top->_frameBufferInfo, 0, true);
            GSP::SetFrameBufferInfo(Bottom->_frameBufferInfo, 1, true);
        }
    }

    void    ScreenImpl::ApplyFading(void)
    {
        Top->Fade(0.5f);
        Bottom->Fade(0.5f);

        Top->SwapBuffer();
        Bottom->SwapBuffer();

        GSP::WaitBufferSwapped(3);

        Top->Copy();
        Bottom->Copy();
    }

    u32     ScreenImpl::AcquireFromGsp(void)
    {
        return Top->Acquire() | Bottom->Acquire();
    }

    u32     ScreenImpl::CheckGspFrameBuffersInfo()
    {
        return Top->ImportFromGsp() | Bottom->ImportFromGsp();
    }

    /*
    ** Swap buffers
    *****************/

    void    ScreenImpl::SwapBuffer(void)
    {
        svcFlushDataCacheRange(GetLeftFrameBuffer(), GetFrameBufferSize());

        GSP::SwapBuffer(!_isTopScreen);

        _currentBuffer = !_currentBuffer;
    }

    void    ScreenImpl::SwapBufferInternal(void)
    {
        _currentBuffer = !_currentBuffer;
    }

    u32     ScreenImpl::GetBacklight(void)
    {
        return REG32(0x10202200 + _backlightOffset);
    }

    void    ScreenImpl::SetBacklight(u32 value)
    {
        REG32(0x10202200 + _backlightOffset) = value;
    }

    GSPGPU_FramebufferFormats   ScreenImpl::GetFormat(void) const
    {
        return _format;
    }

    u16     ScreenImpl::GetWidth(void) const
    {
        return _width;
    }

    u32     ScreenImpl::GetStride(void) const
    {
        return _stride;
    }

    u32     ScreenImpl::GetRowSize(void) const
    {
        return _rowSize;
    }

    u32     ScreenImpl::GetBytesPerPixel(void) const
    {
        return _bytesPerPixel;
    }

    u32     ScreenImpl::GetFrameBufferSize(void) const
    {
        return _stride * _width;
    }

    void    ScreenImpl::GetFrameBufferInfos(int &rowStride, int &bpp, GSPGPU_FramebufferFormats &format) const
    {
        rowStride = _stride;
        bpp = _bytesPerPixel;
        format = _format;
    }

    /*
    ** Framebuffers Getters
    ******************************/

    /*
    ** Left
    *************/
    u8      *ScreenImpl::GetLeftFrameBuffer(bool current)
    {
        return reinterpret_cast<u8 *>(_leftFrameBuffers[current ? _currentBuffer : !_currentBuffer]);
    }

    u8      *ScreenImpl::GetLeftFrameBuffer(int posX, int posY)
    {

        posX = std::max(posX, 0);
        posX = std::min(posX, (_isTopScreen ? 400 : 320));
        posY = std::max(posY, 0);
        posY = std::min(posY, 240);

        // Correct posY
        posY += _rowSize - 240;

        u32 offset = (_rowSize - 1 - posY + posX * _rowSize) * _bytesPerPixel;

        return reinterpret_cast<u8 *>(_leftFrameBuffers[!_currentBuffer]) + offset;
    }

    /*
    ** Right
    *************/

    u8      *ScreenImpl::GetRightFrameBuffer(bool current)
    {
        if (!_isTopScreen)
            return (nullptr);

        return reinterpret_cast<u8 *>(_rightFrameBuffers[current ? _currentBuffer : !_currentBuffer]);
    }

    u8      *ScreenImpl::GetRightFrameBuffer(int posX, int posY)
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

        return reinterpret_cast<u8 *>(_rightFrameBuffers[!_currentBuffer]) + offset;
    }

    void    ScreenImpl::GetPosFromAddress(u32 addr, int &posX, int &posY)
    {
        addr -= _leftFrameBuffers[!_currentBuffer];

        posX = addr / (_rowSize * _bytesPerPixel);
        posY = _rowSize - 1 - ((addr / _bytesPerPixel) - (_rowSize * posX));
        posY -= _rowSize - 240;
    }
}

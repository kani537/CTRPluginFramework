#include "3DS.h"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFrameworkImpl/arm11kCommands.h"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

#include <cstdio>
#include <cstring>
#include "ctrulib/gpu/gpu.h"
#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"

extern 		Handle gspThreadEventHandle;

namespace CTRPluginFramework
{
	u32         ProcessImpl::_processID = 0;
	u64         ProcessImpl::_titleID = 0;
	char        ProcessImpl::_processName[8] = {0};
	u32         ProcessImpl::_kProcess = 0;
	u32         ProcessImpl::_kProcessState = 0;
    u32         ProcessImpl::mmuTable = 0;
    u32         ProcessImpl::mmuTableSize = 0;
	KCodeSet    ProcessImpl::_kCodeSet = {0};
	Handle      ProcessImpl::_processHandle = 0;
	Handle      ProcessImpl::_mainThreadHandle = 0;
    Handle      ProcessImpl::FrameEvent = 0;
    RecursiveLock   ProcessImpl::FrameLock;
	u32         ProcessImpl::_isPaused = 0;
	bool        ProcessImpl::_isAcquiring = false;

    // pluginInit.cpp
    extern      Handle      g_keepEvent;

	void    ProcessImpl::Initialize(void)
	{
		char    kproc[0x100] = {0};
		bool 	isNew3DS = System::IsNew3DS();

		// Get current KProcess
		_kProcess = (u32)arm11kGetCurrentKProcess();

		// Copy KProcess data
		arm11kMemcpy((u32)&kproc, _kProcess, 0x100);
		if (isNew3DS)
		{
			// Copy KCodeSet
			arm11kMemcpy((u32)&_kCodeSet, *(u32 *)(kproc + 0xB8), sizeof(KCodeSet));

			// Copy process id
			_processID = *(u32 *)(kproc + 0xBC);
			_kProcessState = _kProcess + 0x88;

            // Get mmutable
            mmuTableSize = *(u32 *)(kproc + 0x1C + 0x44);
            mmuTable = *(u32 *)(kproc + 0x1C + 0x48);
		}
		else
		{
			// Copy KCodeSet
			arm11kMemcpy((u32)&_kCodeSet, *(u32 *)(kproc + 0xB0), sizeof(KCodeSet));

			// Copy process id
			_processID = *(u32 *)(kproc + 0xB4);
			_kProcessState = _kProcess + 0x80;

            // Get mmutable
            mmuTableSize = *(u32 *)(kproc + 0x1C + 0x3C);
            mmuTable = *(u32 *)(kproc + 0x1C + 0x40);
		}

		// Copy process name
		for (int i = 0; i < 8; i++)
				_processName[i] = _kCodeSet.processName[i];

		// Copy title id
		_titleID = _kCodeSet.titleId;
		// Create handle for this process
		svcOpenProcess(&_processHandle, _processID);
	}

    void    ProcessImpl::UpdateThreadHandle(void)
    {
        _mainThreadHandle = threadGetCurrent()->handle;
        RecursiveLock_Init(&FrameLock);
        svcCreateEvent(&FrameEvent, RESET_ONESHOT);
        while (R_FAILED(svcSetThreadPriority(_mainThreadHandle, Preferences::Settings.ThreadPriority)));
    }

	bool 	ProcessImpl::IsPaused(void)
	{
		return (_isPaused > 0);
	}

	bool 	ProcessImpl::IsAcquiring(void)
	{
		return (_isAcquiring);
	}

    extern "C" Handle gspThreadEventHandle;;
    extern "C" Handle gspEvent;
    extern "C" bool   IsPaused(void)
    {
        return (ProcessImpl::IsPaused());
    }

	void 	ProcessImpl::Pause(bool useFading)
	{
        // Increase pause counter
        ++_isPaused;

        // If game is already paused, nothing to do
        if (_isPaused > 1)
            return;

        // Wake up gsp event thread
        svcSignalEvent(gspEvent);

        // Wait for the frame to be paused
        OSDImpl::WaitFramePaused();

        if (!useFading)
            return;

        float fade = 0.03f;
        Clock t = Clock();
        Time limit = Seconds(1) / 10.f;
        Time delta;
        float pitch = 0.0006f;

        while (fade <= 0.3f)
        {
        	delta = t.Restart();
        	fade += pitch * delta.AsMilliseconds();

        	ScreenImpl::Top->Fade(fade);
        	ScreenImpl::Bottom->Fade(fade);

        	ScreenImpl::Top->SwapBuffer(true, true);
        	ScreenImpl::Bottom->SwapBuffer(true, true);
        	gspWaitForVBlank();
        	if (System::IsNew3DS())
        		while (t.GetElapsedTime() < limit);
        }
	}

	void 	ProcessImpl::Play(bool useFading)
	{
        // If game isn't paused, abort
        if (!_isPaused)
            return;

		if (useFading)
		{
            Time limit = Seconds(1) / 10.f;
            Time delta;
            float pitch = 0.10f;
            Clock t = Clock();
	        float fade = -0.1f;
            while (fade >= -0.9f)
            {
                delta = t.Restart();
                ScreenImpl::Top->Fade(fade);
                ScreenImpl::Bottom->Fade(fade);
                fade -= 0.001f * delta.AsMilliseconds();
                //Sleep(Milliseconds(10));
                ScreenImpl::Top->SwapBuffer(true, true);
                ScreenImpl::Bottom->SwapBuffer(true, true);
                gspWaitForVBlank();
                if (System::IsNew3DS())
                    while (t.GetElapsedTime() < limit); //<- On New3DS frequencies, the alpha would be too dense
            }
		}
        // Decrease pause counter
        if (_isPaused)
            --_isPaused;

        // Resume frame
        if (!_isPaused)
            OSDImpl::ResumeFrame();
	}

    bool     ProcessImpl::PatchProcess(u32 addr, u8 *patch, u32 length, u8 *original)
    {
 		if (original != nullptr)
 		{
 			if (!Process::CopyMemory((void *)original, (void *)addr, length))
 				goto error;
 		}

 		if (!Process::CopyMemory((void *)addr, (void *)patch, length))
 			goto error;

        return (true);
    error:
        return (false);
    }

    void    ProcessImpl::GetHandleTable(KProcessHandleTable& table, std::vector<HandleDescriptor>& handleDescriptors)
    {
        bool 	isNew3DS = System::IsNew3DS();

        // Copy KProcessHandleTable
        arm11kMemcpy((u32)&table, _kProcess + (isNew3DS ? 0xDC : 0xD4), sizeof(KProcessHandleTable));

        u32 count = table.handlesCount;
        u32 size = sizeof(HandleDescriptor) * count;
        u32 start = (u32)table.handleTable;

        while (count-- > 0)
        {
            HandleDescriptor desc;

            arm11kMemcpy((u32)&desc, start, sizeof(HandleDescriptor));
            handleDescriptors.push_back(desc);

            start += sizeof(HandleDescriptor);
        }
    }
}

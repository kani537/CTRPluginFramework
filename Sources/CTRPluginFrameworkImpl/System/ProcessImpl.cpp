#include "3DS.h"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFrameworkImpl/arm11kCommands.h"

#include <cstdio>
#include <cstring>

extern 		Handle gspThreadEventHandle;

namespace CTRPluginFramework
{
	u32         ProcessImpl::_processID = 0;
	u64         ProcessImpl::_titleID = 0;
	char        ProcessImpl::_processName[8] = {0};
	u32         ProcessImpl::_kProcess = 0;
	u32			ProcessImpl::_kProcessState = 0;
	KCodeSet    ProcessImpl::_kCodeSet = {0};
	Handle 		ProcessImpl::_processHandle = 0;
	Handle 		ProcessImpl::_mainThreadHandle = 0;
	bool 		ProcessImpl::_isPaused = false;
	bool 		ProcessImpl::_isAcquiring = false;

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
		}
		else
		{
			// Copy KCodeSet
			arm11kMemcpy((u32)&_kCodeSet, *(u32 *)(kproc + 0xB0), sizeof(KCodeSet));          

			// Copy process id
			_processID = *(u32 *)(kproc + 0xB4);
			_kProcessState = _kProcess + 0x80;
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
    }

	bool 	ProcessImpl::IsPaused(void)
	{
		return (_isPaused);
	}

	bool 	ProcessImpl::IsAcquiring(void)
	{
		return (_isAcquiring);
	}

    extern "C" Handle g_gspSignalEvent;
    extern "C" Handle gspEvent;
    extern "C" bool   IsPaused(void)
    {
        return (ProcessImpl::IsPaused());
    }

	void 	ProcessImpl::Pause(bool useFading)
	{
        _isPaused = true;

        svcSignalEvent(gspEvent);
		// Raising priority of Event Thread
		while (R_FAILED(svcSetThreadPriority(gspThreadEventHandle, 0x19)));
		// Raising priority of this thread        
		while (R_FAILED(svcSetThreadPriority(_mainThreadHandle, 0x18)));        
		
		// Waking up Init thread
		svcSignalEvent(g_keepEvent);

		_isAcquiring = true;
		Screen::Top->Acquire(false);
        Screen::Bottom->Acquire(false);
		_isAcquiring = false;

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

        	Screen::Top->Fade(fade);
        	Screen::Bottom->Fade(fade);

        	Screen::Top->SwapBuffer(true, true);
        	Screen::Bottom->SwapBuffer(true, true);
        	gspWaitForVBlank(); 
        	if (System::IsNew3DS())
        		while (t.GetElapsedTime() < limit);       	
        }       
	}

	void 	ProcessImpl::Play(bool useFading)
	{	
		Time limit = Seconds(1) / 10.f;
		Time delta;
        float pitch = 0.10f;
        Clock t = Clock();

		if (useFading)
		{
	        float fade = -0.1f;
	        while (fade >= -0.9f)
	        {
	        	delta = t.Restart();
	        	Screen::Top->Fade(fade);
	        	Screen::Bottom->Fade(fade);
	        	fade -= 0.001f * delta.AsMilliseconds();
	        	//Sleep(Milliseconds(10));
	        	Screen::Top->SwapBuffer(true, true);
	        	Screen::Bottom->SwapBuffer(true, true);
	        	gspWaitForVBlank();
	        	if (System::IsNew3DS())
	        	  while (t.GetElapsedTime() < limit); //<- On New3DS frequencies, the alpha would be too dense
	        }			
		}
        _isPaused = false;
		while(R_FAILED(svcSetThreadPriority(gspThreadEventHandle, 0x3F)));
		while(R_FAILED(svcSetThreadPriority(_mainThreadHandle, 0x3E)));
	}

    bool     ProcessImpl::PatchProcess(u32 addr, u8 *patch, u32 length, u8 *original)
    {
        if (!(Process::ProtectMemory(((addr / 0x1000) * 0x1000), 0x1000))) goto error;
 
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

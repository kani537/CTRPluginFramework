#include "CTRPluginFramework.hpp"
#include "arm11kCommands.h"
#include "3DS.h"
#include <cstdio>

namespace CTRPluginFramework
{
	u32         Process::_processID = 0;
	u64         Process::_titleID = 0;
	char        Process::_processName[8] = {0};
	u32         Process::_kProcess = 0;
	u32			Process::_kProcessState = 0;
	KCodeSet    Process::_kCodeSet = {0};
	Handle 		Process::_handle = 0;
	Handle 		Process::_mainThreadHandle = 0;
	//u32         *Process::_kProcessHandleTable = nullptr;


	void    Process::Initialize(Handle threadHandle, bool isNew3DS)
	{
		char    kproc[0x100] = {0};

		// Get current KProcess
		_kProcess = (u32)arm11kGetCurrentKProcess();
		_kProcessState = _kProcess + 0x88;
		// Copy KProcess data
		arm11kMemcpy((u32)&kproc, _kProcess, 0x100);
		if (isNew3DS)
		{
			// Copy KCodeSet
			arm11kMemcpy((u32)&_kCodeSet, *(u32 *)(kproc + 0xB8), sizeof(KCodeSet));          

			// Copy process id
			_processID = *(u32 *)(kproc + 0xBC);
		}
		else
		{
			// Copy KCodeSet
			arm11kMemcpy((u32)&_kCodeSet, *(u32 *)(kproc + 0xB0), 0x64);          

			// Copy process id
			_processID = *(u32 *)(kproc + 0xB4);
		}

		// Copy process name
		for (int i = 0; i < 8; i++)
				_processName[i] = _kCodeSet.processName[i];

		// Copy title id
		_titleID = _kCodeSet.titleId;
		// Create handle for this process
		svcOpenProcess(&_handle, _processID);
		_mainThreadHandle = threadHandle;
	}

	Handle 	Process::GetHandle(void)
	{
		return (_handle);
	}

	u32     Process::GetProcessID(void)
	{
		return (_processID);
	}

	void     Process::GetProcessID(char *output)
	{
		if (!output)
			return;
		sprintf(output, "%02X", _processID);
	}

	u64     Process::GetTitleID(void)
	{
		return (_titleID);
	}

	void     Process::GetTitleID(char *output)
	{
		if (!output)
			return;
		sprintf(output, "%016llX", _titleID);
	}

	void    Process::GetName(char *output)
	{
		if (output != nullptr)
			for (int i = 0; i < 8; i++)
				output[i] = _processName[i];
	}

	u8 		Process::GetProcessState(void)
	{
		return (arm11kGetKProcessState(_kProcessState));
	}

	void 	Process::Pause(void)
	{
		svcSetThreadPriority(_mainThreadHandle, 0x18);
	}

	void 	Process::Play(void)
	{
		svcSetThreadPriority(_mainThreadHandle, 0x3F);
	}
}
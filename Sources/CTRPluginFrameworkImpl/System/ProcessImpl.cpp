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
	Handle      ProcessImpl::ProcessHandle = 0;
	u32         ProcessImpl::IsPaused = 0;
    u32         ProcessImpl::ProcessId = 0;
    u64         ProcessImpl::TitleId = 0;

    KThread *   ProcessImpl::MainThread;
    KProcess *  ProcessImpl::KProcessPtr;
    KCodeSet    ProcessImpl::CodeSet;

	void    ProcessImpl::Initialize(void)
	{
		char    kproc[0x100] = {0};
		bool 	isNew3DS = System::IsNew3DS();

		// Get current KProcess
		KProcessPtr = KProcess::GetCurrent();

		// Copy KProcess data
		Kernel::Memcpy(kproc, KProcessPtr, 0x100);

		if (isNew3DS)
		{
			// Copy KCodeSet
			Kernel::Memcpy(&CodeSet, (void *)*(u32 *)(kproc + 0xB8), sizeof(KCodeSet));

			// Copy process id
			ProcessId = *(u32 *)(kproc + 0xBC);

            // Get main thread
            MainThread = (KThread *)*(u32 *)(kproc + 0xC8);

            // Patch KProcess to allow creating threads on Core2
            KProcessPtr->PatchCore2Access();
		}
		else
		{
			// Copy KCodeSet
			Kernel::Memcpy(&CodeSet, (void *)*(u32 *)(kproc + 0xB0), sizeof(KCodeSet));

			// Copy process id
			ProcessId = *(u32 *)(kproc + 0xB4);

            // Get main thread
            MainThread = (KThread *)*(u32 *)(kproc + 0xC0);
		}

		// Copy title id
		TitleId = CodeSet.titleId;

		// Create handle for this process
		svcOpenProcess(&ProcessHandle, ProcessId);
	}

    extern "C" Handle gspEvent;
    extern "C" bool   IsPaused(void)
    {
        return (ProcessImpl::IsPaused > 0);
    }

	void 	ProcessImpl::Pause(bool useFading)
	{
        // Increase pause counter
        ++IsPaused;

        // If game is already paused, nothing to do
        if (IsPaused > 1)
            return;

        // Wake up gsp event thread
        svcSignalEvent(gspEvent);

        // Wait for the frame to be paused
        OSDImpl::WaitFramePaused();

        // Wait for the vblank
        gspWaitForVBlank();
        gspWaitForVBlank1();

        // Acquire screens
        ScreenImpl::Bottom->Acquire();
        ScreenImpl::Top->Acquire();
        OSDImpl::UpdateScreens();

        if (!useFading)
            return;

        ScreenImpl::ApplyFading();
	}

	void 	ProcessImpl::Play(bool useFading)
	{
        // If game isn't paused, abort
        if (!IsPaused)
            return;

        // Decrease pause counter
        if (IsPaused)
            --IsPaused;

        // Resume frame
        if (!IsPaused)
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
        Kernel::Memcpy(&table, (void *)((u32)KProcessPtr + (isNew3DS ? 0xDC : 0xD4)), sizeof(KProcessHandleTable));

        u32 count = table.handlesCount;

        handleDescriptors.resize(count);
        Kernel::Memcpy(handleDescriptors.data(), table.handleTable, count * sizeof(HandleDescriptor));
    }

    void    ProcessImpl::GetGameThreads(std::vector<KThread *> &threads)
    {
        threads.clear();

        KProcessHandleTable             table;
        std::vector<HandleDescriptor>   handles;

        GetHandleTable(table, handles);

        threads.push_back(MainThread);

        for (HandleDescriptor &handle : handles)
        {
            if (!handle.obj->IsKThread())
                continue;

            KThread *thread = reinterpret_cast<KThread *>(handle.obj);

            if (!thread->IsPluginThread())
                threads.push_back(thread);
        }
    }

    void    ProcessImpl::LockGameThreads(void)
    {
        std::vector<KThread *>  threads;

        GetGameThreads(threads);

        for (KThread *thread : threads)
            thread->Lock();
    }

    void    ProcessImpl::UnlockGameThreads(void)
    {
        std::vector<KThread *>  threads;

        GetGameThreads(threads);

        for (KThread *thread : threads)
            thread->Unlock();
    }
}

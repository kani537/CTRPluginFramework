#include "3DS.h"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"

#include <cstdio>
#include <cstring>
#include "ctrulib/gpu/gpu.h"
#include "csvc.h"

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

    std::vector<MemInfo> ProcessImpl::MemRegions;

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

        // Acquire screens
        ScreenImpl::Bottom->Acquire();
        ScreenImpl::Top->Acquire();
        OSDImpl::UpdateScreens();

        // Update memregions
        UpdateMemRegions();

        if (!useFading)
            return;

        ScreenImpl::ApplyFading();
	}

	void 	ProcessImpl::Play(bool forced)
	{
        // If game isn't paused, abort
        if (!IsPaused)
            return;

        // Decrease pause counter
        --IsPaused;

        // Force play (reset counter) if requested
        if (forced)
            IsPaused = 0;

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
            if (!(handle.obj->GetType() == KType::KThread))
                continue;

            KThread *thread = reinterpret_cast<KThread *>(handle.obj);

            if (!thread->IsPluginThread())
                threads.push_back(thread);
        }
    }

    void    ProcessImpl::LockGameThreads(void)
    {
        // !! NOT WORKING !!
        std::vector<KThread *>  threads;

        GetGameThreads(threads);

        for (KThread *thread : threads)
            thread->Lock();
    }

    void    ProcessImpl::UnlockGameThreads(void)
    {
        // !! NOT WORKING !!
        std::vector<KThread *>  threads;

        GetGameThreads(threads);

        for (KThread *thread : threads)
            thread->Unlock();
    }

    static bool     IsInRegion(MemInfo &memInfo, u32 addr)
    {
        addr -= memInfo.base_addr;
        return addr < memInfo.size;
    }

    extern "C" u32 __ctru_linear_heap;

#define MEMPERM_RW (MEMPERM_READ | MEMPERM_WRITE)

    void    ProcessImpl::UpdateMemRegions(void)
    {
        MemRegions.clear();

        bool    regionPatched  = false;

        for (u32 addr = 0x00100000; addr < 0x40000000; )
        {
            MemInfo     memInfo;
            PageInfo    pageInfo;

            if (R_SUCCEEDED(svcQueryProcessMemory(&memInfo, &pageInfo, ProcessHandle, addr)))
            {
                // If region is FREE, IO, SHARED or LOCKED, skip it
                if (memInfo.state == MEMSTATE_FREE || memInfo.state == MEMSTATE_IO
                    || memInfo.state == MEMSTATE_LOCKED || memInfo.state == MEMSTATE_SHARED)
                {
                    addr = memInfo.base_addr + memInfo.size;
                    continue;
                }

                // Same if the memregion is part of CTRPF or NTR
                if (memInfo.base_addr == 0x06000000 || memInfo.base_addr == 0x07000000
                    || memInfo.base_addr == 0x01E80000 || IsInRegion(memInfo, __ctru_linear_heap))
                {
                    addr = memInfo.base_addr + memInfo.size;
                    continue;
                }

                // Check if the region needs patching
                if ((memInfo.perm & MEMPERM_RW) != MEMPERM_RW)
                {
                    u32 perm = memInfo.perm | MEMPERM_RW;

                    regionPatched |=
                        R_SUCCEEDED(svcControlProcessMemory(ProcessImpl::ProcessHandle,
                            memInfo.base_addr, 0, memInfo.size, MEMOP_PROT, perm));
                }

                // Add it to the vector if necessary
                if (!regionPatched)
                    MemRegions.push_back(memInfo);

                addr = memInfo.base_addr + memInfo.size;
                continue;
            }

            addr += 0x1000;
        }

        if (regionPatched)
            UpdateMemRegions();
    }

    MemInfo    ProcessImpl::GetMemRegion(u32 address)
    {
        for (MemInfo &memInfo : MemRegions)
            if (IsInRegion(memInfo, address))
                return memInfo;

        // Not found return an empty region
        return MemInfo{0, 0, 0, 0};
    }

    MemInfo     ProcessImpl::GetNextRegion(const MemInfo &region)
    {
        for (MemInfo &memInfo : MemRegions)
            if (memInfo > region)
                return memInfo;

        return region;
    }

    MemInfo     ProcessImpl::GetPreviousRegion(const MemInfo &region)
    {
        MemInfo *prev = nullptr;

        for (MemInfo &memInfo : MemRegions)
        {
            if (memInfo >= region)
                return prev != nullptr ? *prev : memInfo;

            prev = &memInfo;
        }

        return region;
    }
}

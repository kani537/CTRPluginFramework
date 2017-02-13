
#include "3DS.h"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFramework/System.hpp"

#include <cstdio>
#include <cstring>

extern 		Handle gspThreadEventHandle;

namespace CTRPluginFramework
{
	Handle 	Process::GetHandle(void)
	{
		return (ProcessImpl::_processHandle);
	}

	u32     Process::GetProcessID(void)
	{
		return (ProcessImpl::_processID);
	}

	void     Process::GetProcessID(char *output)
	{
		if (!output)
			return;
		sprintf(output, "%02X", ProcessImpl::_processID);
	}

	u64     Process::GetTitleID(void)
	{
		return (ProcessImpl::_titleID);
	}

	void     Process::GetTitleID(char *output)
	{
		if (!output)
			return;
		sprintf(output, "%016llX", ProcessImpl::_titleID);
	}

	void    Process::GetName(char *output)
	{
		if (output != nullptr)
			for (int i = 0; i < 8; i++)
				output[i] = ProcessImpl::_processName[i];
	}

	bool 	Process::Patch(u32 	addr, u8 *patch, u32 length, u8 *original)
	{
		return (ProcessImpl::PatchProcess(addr, patch, length, original));
	}

    bool     Process::ProtectMemory(u32 addr, u32 size, int perm)
    {
    	if (R_FAILED(svcControlProcessMemory(ProcessImpl::_processHandle, addr, addr, size, 6, perm)))
        	return (false);
        return (true);
    }

    bool     Process::ProtectRegion(u32 addr, int perm)
    {
    	MemInfo 	minfo;
    	PageInfo 	pinfo;

    	if (R_FAILED(svcQueryProcessMemory(&minfo, &pinfo, ProcessImpl::_processHandle, addr))) goto error;
    	if (minfo.state == MEMSTATE_FREE) goto error;
    	if (addr < minfo.base_addr || addr > minfo.base_addr + minfo.size) goto error;

    	return (ProtectMemory(minfo.base_addr, minfo.size, perm));
    error:
        return (false);
    }

    bool     Process::CopyMemory(void *dst, void *src, u32 size)
    {
        if (R_FAILED(svcFlushProcessDataCache(ProcessImpl::_processHandle, src, size))) goto error;
        if (R_FAILED(svcFlushProcessDataCache(ProcessImpl::_processHandle, dst, size))) goto error;
        std::memcpy(dst, src, size);
        svcInvalidateProcessDataCache(ProcessImpl::_processHandle, dst, size);
        return (true);
    error:
        return (false);
    }
}

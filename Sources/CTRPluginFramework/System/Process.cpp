
#include "3DS.h"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
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

	u64     Process::GetTitleID(void)
	{
		return (ProcessImpl::_titleID);
	}

    void    Process::GetTitleID(std::string &output)
    {
        char tid[17] = { 0 };

        sprintf(tid, "%016llX", ProcessImpl::_titleID);
        for (int i = 0; i < 16; i++)
            output += tid[i];
    }

    void    Process::GetName(std::string &output)
	{
	    for (int i = 0; i < 8; i++)
		    output += ProcessImpl::_processName[i];
	}

    u16     Process::GetVersion(void)
    {
        AM_TitleEntry   entry = { 0 };
        AM_TitleEntry   entryUpd = { 0 };
        AM_TitleEntry   entryCard = { 0 };

        u64  tid = Process::GetTitleID();
        u64  tidupdate = tid | 0x000000E00000000;

        bool res = R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_SD, 1, &tid, &entry));
        bool resUpd = R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_SD, 1, &tidupdate, &entryUpd));
        bool resCard = R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_GAME_CARD, 1, &tid, &entryCard));

        if (resCard)
        {
            if (resUpd)
                return (std::max(entryUpd.version, entryCard.version));
            return (entryCard.version);
        }

        if (resUpd)
            return (entryUpd.version);      

        return (entry.version);
    }

    u32     Process::GetTextSize(void)
    {
        return (ProcessImpl::_kCodeSet.textPages * 0x1000);
    }

    bool    Process::IsPaused(void)
	{
        return (ProcessImpl::_isPaused);
	}

    bool 	Process::Patch(u32 	addr, void *patch, u32 length, void *original)
	{
		return (ProcessImpl::PatchProcess(addr, static_cast<u8 *>(patch), length, static_cast<u8 *>(original)));
	}

    bool    Process::Patch(u32 addr, u32 patch, void *original)
    {
        return (ProcessImpl::PatchProcess(addr, reinterpret_cast<u8 *>(&patch), 4, static_cast<u8 *>(original)));
    }

    bool     Process::ProtectMemory(u32 addr, u32 size, int perm)
    {
        if (addr & 0xFFF)
        {
            addr  &= ~0xFFF;
            size  += 0x1000;
            size &= ~0xFFF;            
        }
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

    void     Process::ProtectRegionInRange(u32 startAddress, u32 endAddress, int perm)
    {
        MemInfo     minfo;
        PageInfo    pinfo;

        while (startAddress < endAddress)
        {
            if (R_SUCCEEDED(svcQueryProcessMemory(&minfo, &pinfo, ProcessImpl::_processHandle, startAddress)))
            {
                if (minfo.state != MEMSTATE_FREE)
                {
                    if (startAddress >= minfo.base_addr && startAddress <= minfo.base_addr + minfo.size)
                    {   
                        if (ProtectMemory(minfo.base_addr, minfo.size, perm))
                        {
                            startAddress += minfo.size;
                            continue;
                        }
                    }
                }
            }
            startAddress += 0x1000;
        }
    }

    bool     Process::CopyMemory(void *dst, void *src, u32 size)
    {
        if (!CheckAddress((u32)src)) goto error;
        if (!CheckAddress((u32)dst)) goto error;

        svcFlushProcessDataCache(ProcessImpl::_processHandle, src, size);
        svcFlushProcessDataCache(ProcessImpl::_processHandle, dst, size);

        std::memcpy(dst, src, size);

        svcInvalidateProcessDataCache(ProcessImpl::_processHandle, dst, size);

        return (true);
    error:
        return (false);
    }

    bool    Process::CheckAddress(u32 address, u32 perm)
    {
        Result         res;
        PageInfo       pInfo = {0};
        MemInfo        mInfo = {0};

        res = svcQueryProcessMemory(&mInfo, &pInfo, ProcessImpl::_processHandle, address);
        if (R_SUCCEEDED(res) && mInfo.base_addr <= address && mInfo.base_addr + mInfo.size > address)
        {
            if ((mInfo.perm & perm) != perm)
            {
                perm |= mInfo.perm;
                res = svcControlProcessMemory(ProcessImpl::_processHandle, mInfo.base_addr, mInfo.base_addr, mInfo.size, 6, perm);
                if (R_SUCCEEDED(res))
                    return (true);
                else
                    return (false);
            }
            return (true);
        }
        return (false);
    }

    bool    Process::CheckRegion(u32 address, u32 &size, u32 perm)
    {
        Result         res;
        PageInfo       pInfo = { 0 };
        MemInfo        mInfo = { 0 };

        res = svcQueryProcessMemory(&mInfo, &pInfo, ProcessImpl::_processHandle, address);
        if (R_SUCCEEDED(res) && mInfo.base_addr <= address && mInfo.base_addr + mInfo.size > address)
        {
            size = mInfo.size;

            if ((mInfo.perm & perm) != perm)
            {
                perm |= mInfo.perm;
                res = svcControlProcessMemory(ProcessImpl::_processHandle, mInfo.base_addr, mInfo.base_addr, mInfo.size, 6, perm);
                if (R_SUCCEEDED(res))
                    return (true);
                return (false);
            }
            return (true);
        }
        return (false);
    }

    bool    Process::Write64(u32 address, u64 value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        {
            *(u64 *)address = value;
            return (true);
        }
        return (false);
    }

    bool    Process::Write32(u32 address, u32 value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        {
            *(u32 *)address = value;
            return (true);
        }
        return (false);
    }

    bool    Process::Write16(u32 address, u16 value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        {
            *(u16 *)address = value;
            return (true);
        }
        return (false);
    }

    bool    Process::Write8(u32 address, u8 value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        {
            *(u8 *)address = value;
            return (true);
        }
        return (false);
    }

    bool    Process::WriteFloat(u32 address, float value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        {
            *(float *)address = value;
            return (true);
        }
        return (false);
    }

    bool    Process::WriteDouble(u32 address, double value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        {
            *(double *)address = value;
            return (true);
        }
        return (false);
    }

    bool    Process::Read64(u32 address, u64 &value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        {
            value = *(u64 *)address;
            return (true);
        }
        return (false);
    }

    bool    Process::Read32(u32 address, u32 &value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        {
            value = *(u32 *)address;
            return (true);
        }
        return (false);
    }

    bool    Process::Read16(u32 address, u16 &value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        {
            value = *(u16 *)address;
            return (true);
        }
        return (false);
    }

    bool    Process::Read8(u32 address, u8 &value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        {
            value = *(u8 *)address;
            return (true);
        }
        return (false);
    }

    bool    Process::ReadFloat(u32 address, float &value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        {
            value = *(float *)address;
            return (true);
        }
        return (false);
    }

    bool    Process::ReadDouble(u32 address, double& value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        {
            value = *(double *)address;
            return (true);
        }
        return (false);
    }
}

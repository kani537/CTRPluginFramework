
#include "3DS.h"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "csvc.h"
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

        AM_GetTitleInfo(MEDIATYPE_SD, 1, &tid, &entry);
        AM_GetTitleInfo(MEDIATYPE_SD, 1, &tidupdate, &entryUpd);

        if (R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_GAME_CARD, 1, &tid, &entryCard)))
            return (std::max(entryUpd.version, entryCard.version));

        return (std::max(entryUpd.version, entry.version));
    }

    u32     Process::GetTextSize(void)
    {
        return (ProcessImpl::_kCodeSet.textPages * 0x1000);
    }

    bool    Process::IsPaused(void)
	{
        return (ProcessImpl::_isPaused);
	}

    void    Process::Pause(void)
    {
        ProcessImpl::Pause(false);
    }

    void    Process::Play(void)
	{
        ProcessImpl::Play(false);
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

    bool     Process::CopyMemory(void *dst, const void *src, u32 size)
    {
        if (!CheckAddress((u32)src)) goto error;
        if (!CheckAddress((u32)dst)) goto error;

        svcFlushProcessDataCache(ProcessImpl::_processHandle, src, size);
        svcInvalidateProcessDataCache(ProcessImpl::_processHandle, dst, size);

        std::memcpy(dst, src, size);

        svcFlushProcessDataCache(ProcessImpl::_processHandle, dst, size);

        return (true);
    error:
        return (false);
    }

    bool    Process::CheckAddress(u32 address, u32 perm)
    {
        Result         res;
        PageInfo       pInfo = {0};
        MemInfo        mInfo = {0};

        res = svcQueryMemory(&mInfo, &pInfo, address);
        if (R_SUCCEEDED(res) && mInfo.base_addr <= address && mInfo.base_addr + mInfo.size > address)
        {
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

    bool    Process::CheckRegion(u32 address, u32 &size, u32 perm)
    {
        Result         res;
        PageInfo       pInfo = { 0 };
        MemInfo        mInfo = { 0 };

        res = svcQueryMemory(&mInfo, &pInfo, address);
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

    union Type32
	{
        u32     bits32;
        u8      bytes[4];
	};

    struct Type64
    {
        Type32  low;
        Type32  high;
    };

    bool    Process::Write64(u32 address, u64 value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            *(u64 *)address = value;
            return (true);
        }
        return (false);
    }

    bool    Process::Write32(u32 address, u32 value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            // Address is aligned
            if (address & 3 == 0)
                *reinterpret_cast<u32 *>(address) = value;
            else
            {
                // Unaligned address
                Type32  type;

                type.bits32 = value;
                *reinterpret_cast<u8 *>(address++) = type.bytes[0];
                *reinterpret_cast<u8 *>(address++) = type.bytes[1];
                *reinterpret_cast<u8 *>(address++) = type.bytes[2];
                *reinterpret_cast<u8 *>(address) = type.bytes[3];
            }
            return (true);
        }
        return (false);
    }

    bool    Process::Write16(u32 address, u16 value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            // Aligned address
            if (address & 1 == 0)
                *reinterpret_cast<u16 *>(address) = value;
            else
            {
                // Unaligned address
                Type32  type;

                type.bits32 = value;
                *reinterpret_cast<u8 *>(address++) = type.bytes[0];
                *reinterpret_cast<u8 *>(address) = type.bytes[1];
            }
            return (true);
        }
        return (false);
    }

    bool    Process::Write8(u32 address, u8 value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            *reinterpret_cast<u8 *>(address) = value;
            return (true);
        }
        return (false);
    }

    bool    Process::WriteFloat(u32 address, float value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            *reinterpret_cast<float *>(address) = value;
            return (true);
        }
        return (false);
    }

    bool    Process::WriteDouble(u32 address, double value)
    {
        if (CheckAddress(address, MEMPERM_WRITE))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            *(double *)address = value;
            return (true);
        }
        return (false);
    }

    bool    Process::Read64(u32 address, u64 &value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            value = *(u64 *)address;
            return (true);
        }
        return (false);
    }

    bool    Process::Read32(u32 address, u32 &value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            // Aligned address
            if (address & 3 == 0)
                value = *reinterpret_cast<vu32 *>(address);
            else
            {
                // Unaligned address
                Type32  type;

                type.bytes[0] = *reinterpret_cast<vu8 *>(address++);
                type.bytes[1] = *reinterpret_cast<vu8 *>(address++);
                type.bytes[2] = *reinterpret_cast<vu8 *>(address++);
                type.bytes[3] = *reinterpret_cast<vu8 *>(address);
                value = type.bits32;
            }
            return (true);
        }
        return (false);
    }

    bool    Process::Read16(u32 address, u16 &value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            // Aligned address
            if (address & 1 == 0)
                value = *reinterpret_cast<vu16 *>(address);
            else
            {
                // Unaligned address
                Type32  type = { 0 };

                type.bytes[0] = *reinterpret_cast<vu8 *>(address++);
                type.bytes[1] = *reinterpret_cast<vu8 *>(address);
                value = static_cast<u16>(type.bits32);
            }
            return (true);
        }
        return (false);
    }

    bool    Process::Read8(u32 address, u8 &value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            value = *reinterpret_cast<vu8 *>(address);
            return (true);
        }
        return (false);
    }

    bool    Process::ReadFloat(u32 address, float &value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            value = *reinterpret_cast<volatile float *>(address);
            return (true);
        }
        return (false);
    }

    bool    Process::ReadDouble(u32 address, double& value)
    {
        if (CheckAddress(address, MEMPERM_READ))
        //if ((address = PA_FROM_VA(address)) != 0)
        {
            value = *reinterpret_cast<volatile double *>(address);
            return (true);
        }
        return (false);
    }
}

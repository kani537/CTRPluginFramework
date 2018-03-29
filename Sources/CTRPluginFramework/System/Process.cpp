
#include "3DS.h"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "csvc.h"
#include <cstdio>
#include <cstring>
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "CTRPluginFrameworkImpl/Graphics/OSDImpl.hpp"

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
        {
            char c = ProcessImpl::_processName[i];
            if (c)
		        output += c;
        }
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
        return (ProcessImpl::_isPaused > 0);
	}

    void    Process::Pause(void)
    {
        ProcessImpl::Pause(false);
    }

    void    Process::Play(const u32 frames)
	{
            if (frames)
            {
                OSDImpl::FramesToPlay = frames;
                RecursiveLock_Unlock(&ProcessImpl::FrameLock);
                RecursiveLock_Lock(&ProcessImpl::FrameLock);
            }
            else
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
        if (!CheckAddress((u32)dst + size)) goto error;
        if (!CheckAddress((u32)src + size)) goto error;

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

    static bool     ConvertString(void *output, const u8 *input, u32 size, StringFormat outfmt)
    {
        if (outfmt == StringFormat::Utf16)
        {
            u16     buffer[0x10];
            u16     *out = reinterpret_cast<u16 *>(output);
            u16     *buf;
            u32     code;
            int     units;

            size >>= 1;
            size <<= 1;
            do
            {
                buf = buffer;
                units = decode_utf8(&code, input);
                if (units == -1)
                    return (false);

                input += units;
                units = encode_utf16(buf, code);
                size -= units;
                if (!size)
                    *out = 0;
                else while (units--)
                    *out++ = *buf++;
            } while (size && code > 0);
        }
        else
        {
            u32     *out = reinterpret_cast<u32 *>(output);
            u32     code;
            int     units;

            size >>= 2;
            size <<= 2;
            do
            {
                units = decode_utf8(&code, input);
                if (units == -1)
                    return (false);

                input += units;
                size -= 4;
                if (!size)
                    *out = 0;
                else while (units--)
                    *out++ = code;

            } while (size && code > 0);
        }

        return (true);
    }

    bool    Process::ReadString(u32 address, std::string &output, u32 size, StringFormat format)
    {
        if (!CheckAddress(address, MEMPERM_READ))
            return (false);

        u8      buffer[0x10];

        if (format == StringFormat::Utf8)
        {
            u32     code = 0;
            u8      *p = reinterpret_cast<u8 *>(address);
            int     unit = 0;

            do
            {
                unit = decode_utf8(&code, p);

                if (unit == -1)
                    return (false);

                size -= unit;

                if (code > 0)
                    while (unit--)
                        output += *p++;
            } while (code > 0 && size);
        }
        else if (format == StringFormat::Utf16)
        {
            u32     code = 0;
            u16     *p = reinterpret_cast<u16 *>(address);
            u8      *buf;
            int     unit = 0;

            size >>= 1;
            size <<= 1;
            do
            {
                buf = buffer;

                unit = decode_utf16(&code, p);
                if (unit == -1)
                    return (false);

                p += unit;
                size -= unit * 2;
                unit = encode_utf8(buf, code);
                if (unit == -1)
                    return (false);

                if (code > 0)
                    while (unit--)
                        output += *buf++;
            } while (code > 0 && size);
        }
        else
        {
            u32     code = 0;
            u32     *p = reinterpret_cast<u32 *>(address);
            u8      *buf;
            int     unit = 0;

            size >>= 2;
            size <<= 2;
            do
            {
                buf = buffer;
                code = *p++;
                size -= 4;

                unit = encode_utf8(buf, code);
                if (unit == -1)
                    return (false);

                if (code > 0)
                    while (unit--)
                        output += *buf++;
            } while (code > 0 && size);
        }

        return (true);
    }

    bool    Process::WriteString(u32 address, const std::string &input, StringFormat outFmt)
    {
        if (!CheckAddress(address, MEMPERM_READ | MEMPERM_WRITE) || input.empty())
            return (false);

        if (outFmt == StringFormat::Utf8)
        {
            u8  *p = reinterpret_cast<u8 *>(address);

            for (char c : input)
            {
                *p++ = c;
            }
            return (true);
        }
        else if (outFmt == StringFormat::Utf16)
        {
            u32         size = (input.size() + 1) * 2;
            const u8    *in = reinterpret_cast<const u8 *>(input.c_str());
            u16         *out = reinterpret_cast<u16 *>(address);

            return (ConvertString(out, in, size, outFmt));
        }

        {
            u32         size = (input.size() + 1) * 4;
            const u8    *in = reinterpret_cast<const u8 *>(input.c_str());
            u32         *out = reinterpret_cast<u32 *>(address);

            return (ConvertString(out, in, size, outFmt));
        }
    }

    bool    Process::WriteString(u32 address, const std::string &input, u32 size, StringFormat outFmt)
    {
        if (!CheckAddress(address, MEMPERM_READ | MEMPERM_WRITE) || input.empty())
            return (false);

        if (outFmt == StringFormat::Utf8)
        {
            u8  *p = reinterpret_cast<u8 *>(address);

            for (char c : input)
            {
                size--;
                if (!size)
                {
                    *p = 0;
                    break;
                }
                *p++ = c;
            }
            return (true);
        }
        else if (outFmt == StringFormat::Utf16)
        {
            const u8    *in = reinterpret_cast<const u8 *>(input.c_str());
            u16         *out = reinterpret_cast<u16 *>(address);

            return (ConvertString(out, in, size, outFmt));
        }

        {
            const u8    *in = reinterpret_cast<const u8 *>(input.c_str());
            u32         *out = reinterpret_cast<u32 *>(address);

            return (ConvertString(out, in, size, outFmt));
        }
    }
}

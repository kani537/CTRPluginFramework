#include "CTRPluginFrameworkImpl/ActionReplay/ARHandler.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFramework/System/Controller.hpp"
#include "CTRPluginFramework/System/Vector.hpp"
#include "CTRPluginFramework/System/Touch.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFrameworkImpl/arm11kCommands.h"
#include "csvc.h"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "ctrulib/result.h"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"

#define debug 0

#if debug
#define Notify(str, ...) OSD::Notify(Utils::Format(str, ##__VA_ARGS__))
#else
#define Notify(str, ...)
#endif

namespace CTRPluginFramework
{
    using Register = ARHandler::Register;

    u32     ARHandler::Offset[2] = { 0 };
    Register    ARHandler::Data[2];
    u32     ARHandler::Storage[2] = { 0 };
    u32     ARHandler::ActiveOffset = 0;
    u32     ARHandler::ActiveData = 0;
    bool    ARHandler::ExitCodeImmediately = false;

    bool    ActionReplay_IsValidAddress(u32 address, bool write)
    {
        if (!Process::CheckAddress(address, (int)write + 1))
        {
            Notify("Unreachable: %08X", address);
            return false;
        }
        return true;
    }

#define CheckAddress(addr, write) ActionReplay_IsValidAddress(addr, write)

    void    ARHandler::Execute(const ARCodeVector& arcodes, u32(&storage)[2])
    {
        Offset[0] = 0;
        Offset[1] = 0;
        Data[0].Clear();
        Data[1].Clear();
        Storage[0] = storage[0];
        Storage[1] = storage[1];
        ActiveOffset = 0;
        ActiveData = 0;
        ExitCodeImmediately = false;
        _Execute(arcodes);
        storage[0] = Storage[0];
        storage[1] = Storage[1];
    }

    bool    Write32(u32 address, u32 value)
    {
        /*u32 pa = svcConvertVAToPA((void *)address, false);

        if (pa == 0 || pa > 0x30000000)
        {
            Notify("AR Write32 Failure: va: %08X, pa: %08X", address, pa);
            return (false);
        }
        *(vu32 *)(pa | (1 << 31)) = value; //arm11kWrite32(pa, value);*/
        if (CheckAddress(address, true))
            *(vu32 *)address = value;
        return (true);
    }

    bool    Write16(u32 address, u16 value)
    {
       /* u32 pa = svcConvertVAToPA((void *)address, false);

        if (pa == 0 || pa > 0x30000000)
        {
            Notify("AR Write16 Failure: va: %08X, pa: %08X", address, pa);
            return (false);
        }
        *(vu16 *)(pa | (1 << 31)) = value; //arm11kWrite32(pa, value); */
        if (CheckAddress(address, true))
            *(vu16 *)address = value;
        return (true);
    }

    bool    Write8(u32 address, u8 value)
    {
       /* u32 pa = svcConvertVAToPA((void *)address, false);

        if (pa == 0 || pa > 0x30000000)
        {
            Notify("AR Write8 Failure: va: %08X, pa: %08X", address, pa);
            return (false);
        }

        *(vu8 *)(pa | (1 << 31)) = value; //arm11kWrite32(pa, value); */
        if (CheckAddress(address, true))
            *(vu8 *)address = value;
        return (true);
    }

    bool    Read32(u32 address, u32 &value)
    {
       /* u32 pa = svcConvertVAToPA((void *)address, false);

        if (pa == 0 || pa > 0x30000000)
        {
            Notify("AR Read32 Failure: va: %08X, pa: %08X", address, pa);
            return (false);
        }

        value = *(vu32 *)(pa | (1 << 31));//arm11kRead32(pa);*/
        if (CheckAddress(address, false))
            value = *(vu32 *)address;
        return (true);
    }

    bool    Read16(u32 address, u16 &value)
    {
        /*u32 pa = svcConvertVAToPA((void *)address, false);

        if (pa == 0 || pa > 0x30000000)
        {
            Notify("AR Read16 Failure: va: %08X, pa: %08X", address, pa);
            return (false);
        }

        value = *(vu16 *)(pa | (1 << 31));//arm11kRead32(pa);*/
        if (CheckAddress(address, false))
            value = *(vu16 *)address;
        return (true);
    }

    bool    Read8(u32 address, u8 &value)
    {
        /*u32 pa = svcConvertVAToPA((void *)address, false);

        if (pa == 0 || pa > 0x30000000)
        {
            Notify("AR Read8 Failure: va: %08X, pa: %08X", address, pa);
            return (false);
        }

        value = *(vu8 *)(pa | (1 << 31));//arm11kRead32(pa);*/
        if (CheckAddress(address, false))
            value = *(vu8 *)address;
        return (true);
    }

    static bool    Memcpy(u32 address, u8 *pattern, u32 size)
    {
        /*u32 pa = svcConvertVAToPA((void *)address, false);

        if (pa == 0 || pa > 0x30000000)
            return (false);

        pa |= (1 << 31);*/
        if (CheckAddress((u32)pattern, false) && CheckAddress(address, true))
            while (size--)
                *(vu8 *)address++ = *pattern++;

        return (true);
    }

    void    ARHandler::_Execute(const ARCodeVector &codes)
    {
        union
        {
            u32         value = 0;
            float       vfpval;
        };
        u16             value16 = 0;
        u16             mask;
        bool            copyMode = false;
        bool            waitForEndLoop = false;
        bool            waitForExitCode = false;
        bool            conditionalMode = false;
        int             conditionCount = 0;
        int             loopCount = 0;
        int             loopIteration = 0;
        ARCodeVector    loopCodes;

        for (const ARCode code : codes)
        {
            Register &currentData = Data[ActiveData];

            // If we must exit
            if (ExitCodeImmediately)
                return;

            // If we have a full terminator
            if (code.Type == 0xD2)
            {
                // If we have blocks waiting to be ended
                if (waitForExitCode)
                {
                    conditionCount = 0;
                    waitForExitCode = false;
                }
                // If we have loops waiting to be launched
                if (waitForEndLoop)
                {
                    loopCount = 0;
                    waitForEndLoop = false;

                    // Execute loops
                    while (loopIteration--)
                        _Execute(loopCodes);
                }
                // Clear registers and continue
                Offset[ActiveOffset] = 0;
                currentData.Clear();
                continue;
            }

            // If we're waiting for the end of a block
            if (waitForExitCode)
            {
                if (code.Type == 0xD0) ///< Terminator code
                {
                    conditionCount--;
                    if (conditionCount == 0)
                        waitForExitCode = false;
                }
                continue;
            }

            // If we're waiting for an execute loop code
            if (waitForEndLoop)
            {
                if (code.Type == 0xD1)
                {
                    loopCount--;
                    if (loopCount == 0)
                    {
                        waitForEndLoop = false;
                        // Execute loop
                        while (loopIteration--)
                            _Execute(loopCodes);
                        continue;
                    }
                }
                loopCodes.push_back(code);
                continue;
            }

            // Execute code
            switch (code.Type)
            {
            case 0x00: ///< Write32
            {
                ExitCodeImmediately = !Write32(code.Left + Offset[ActiveOffset], code.Right);
                break;
            }
            case 0x10: ///< Write16
            {
                ExitCodeImmediately = !Write16(code.Left + Offset[ActiveOffset], code.Right);
                break;
            }
            case 0x20: ///< Write8
            {
                ExitCodeImmediately = !Write8(code.Left + Offset[ActiveOffset], code.Right);
                break;
            }
            case 0x30: ///< GreaterThan 32Bits
            {
                if (!((ExitCodeImmediately = !Read32(code.Left + Offset[ActiveOffset], value))))
                {
                    // If we must compare with data register
                    if (conditionalMode)
                    {
                        // If current data is in vfp mode
                        if (currentData.isVFP)
                        {
                            if (currentData.vfp > vfpval)
                                continue;
                        }
                        else if (currentData.value > value)
                            continue;
                    }
                    else if (code.Right > value)
                        continue;
                }
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x40: ///< LesserThan 32Bits
            {
                if (!((ExitCodeImmediately = !Read32(code.Left + Offset[ActiveOffset], value))))
                {
                    // If we must compare with data register
                    if (conditionalMode)
                    {
                        // If current data is in vfp mode
                        if (currentData.isVFP)
                        {
                            if (currentData.vfp < vfpval)
                                continue;
                        }
                        else if (currentData.value < value)
                            continue;
                    }
                    else if (code.Right < value)
                        continue;
                }
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x50: ///< EqualTo 32Bits
            {
                if (!((ExitCodeImmediately = !Read32(code.Left + Offset[ActiveOffset], value))))
                {
                    // If we must compare with data register
                    if (conditionalMode)
                    {
                        // If current data is in vfp mode
                        if (currentData.isVFP)
                        {
                            if (currentData.vfp == vfpval)
                                continue;
                        }
                        else if (currentData.value == value)
                            continue;
                    }
                    else if (code.Right == value)
                        continue;
                }
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x60: ///< NotEqualTo 32Bits
            {
                if (!((ExitCodeImmediately = !Read32(code.Left + Offset[ActiveOffset], value))))
                {
                    // If we must compare with data register
                    if (conditionalMode)
                    {
                        // If current data is in vfp mode
                        if (currentData.isVFP)
                        {
                            if (currentData.vfp != vfpval)
                                continue;
                        }
                        else if (currentData.value != value)
                            continue;
                    }
                    else if (code.Right != value)
                        continue;
                }
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x70: ///< GreaterThan 16Bits
            {
                mask = code.Right >> 16;

                if (!((ExitCodeImmediately = !Read16(code.Left + Offset[ActiveOffset], value16))))
                {
                    value16 &= ~mask;
                    // If we must compare with data register
                    if (conditionalMode)
                    {
                        if ((currentData.value & 0xFFFF) > value16)
                            continue;
                    }
                    else if ((code.Right & 0xFFFF) > value16)
                        continue;
                }
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x80: ///< LesserThan 16Bits
            {
                mask = code.Right >> 16;

                if (!((ExitCodeImmediately = !Read16(code.Left + Offset[ActiveOffset], value16))))
                {
                    value16 &= ~mask;
                    // If we must compare with data register
                    if (conditionalMode)
                    {
                        if ((currentData.value & 0xFFFF) < value16)
                            continue;
                    }
                    else if ((code.Right & 0xFFFF) < value16)
                        continue;
                }
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x90: ///< EqualTo 16Bits
            {
                mask = code.Right >> 16;

                if (!((ExitCodeImmediately = !Read16(code.Left + Offset[ActiveOffset], value16))))
                {
                    value16 &= ~mask;
                    // If we must compare with data register
                    if (conditionalMode)
                    {
                        if ((currentData.value & 0xFFFF) == value16)
                            continue;
                    }
                    else if ((code.Right & 0xFFFF) == value16)
                        continue;
                }
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0xA0: ///< NotEqualTo 16Bits
            {
                mask = code.Right >> 16;

                if (!((ExitCodeImmediately = !Read16(code.Left + Offset[ActiveOffset], value16))))
                {
                    value16 &= ~mask;
                    // If we must compare with data register
                    if (conditionalMode)
                    {
                        if ((currentData.value & 0xFFFF) != value16)
                            continue;
                    }
                    else if ((code.Right & 0xFFFF) != value16)
                        continue;
                }
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0xB0: ///< Read Offset
            {
                ExitCodeImmediately = !Read32(code.Left + Offset[ActiveOffset], Offset[ActiveOffset]);
                break;
            }
            case 0xD3: ///< Set Offset[ActiveOffset]
            {
                Offset[ActiveOffset] = code.Right;
                break;
            }
            case 0xDC: ///< Add to Offset[ActiveOffset]
            {
                Offset[ActiveOffset] += code.Right;
                break;
            }
            case 0xC0: ///< Loop code
            {
                loopIteration = code.Right;
                loopCount++;
                waitForEndLoop = true;
                break;
            }
            case 0xC1: ///< Loop code data#1
            {
                loopIteration = Data[0].value;
                loopCount++;
                waitForEndLoop = true;
                break;
            }
            case 0xC2: ///< Loop code data#2
            {
                loopIteration = Data[1].value;
                loopCount++;
                waitForEndLoop = true;
                break;
            }
            case 0xD0: ///< Terminator
            {
                break;
            }
            case 0xD2: ///< Full Terminator
            {
                if (code.Right)
                {
                    ExitCodeImmediately = true;
                    return;
                }

                Offset[ActiveOffset] = 0;
                currentData.Clear();
                break;
            }
            case 0xD4: ///< Add to Data[ActiveData]
            {
                value = code.Right;
                if (currentData.isVFP)
                    currentData.vfp += vfpval;
                else
                    currentData.value += value;
                break;
            }
            case 0xD5: ///< Set Data[ActiveData]
            {
                currentData.value = code.Right;
                break;
            }
            case 0xD6: ///< Write 32bits Data[ActiveData]
            {
                ExitCodeImmediately = !Write32(code.Right + Offset[ActiveOffset], currentData.value);
                Offset[ActiveOffset] += 4;
                break;
            }
            case 0xD7: ///< Write 16Bits Data[ActiveData]
            {
                value16 = currentData.value;
                ExitCodeImmediately = !Write16(code.Right + Offset[ActiveOffset], value16);
                Offset[ActiveOffset] += 2;
                break;
            }
            case 0xD8: ///< Write 8Bits Data[ActiveData]
            {
                u8 value8 = currentData.value;
                ExitCodeImmediately = !Write8(code.Right + Offset[ActiveOffset], value8);
                Offset[ActiveOffset]++;
                break;
            }
            case 0xD9: ///< Read 32bits to Data[ActiveData]
            {
                ExitCodeImmediately = !Read32(code.Right + Offset[ActiveOffset], currentData.value);
                break;
            }
            case 0xDA: ///< Read 16Bits to Data[ActiveData]
            {
                ExitCodeImmediately = !Read16(code.Right + Offset[ActiveOffset], value16);
                currentData.value = value16;
                break;
            }
            case 0xDB: ///< Read 8Bits to Data[ActiveData]
            {
                u8 value8 = 0;
                ExitCodeImmediately = !Read8(code.Right + Offset[ActiveOffset], value8);
                currentData.value = value8;
                break;
            }
            case 0xDD: ///< Keypad code
            {
                if (!Controller::IsKeysDown(code.Right))
                {
                    waitForExitCode = true;
                    conditionCount++;
                }
                break;
            }
            case 0xDE: ///< Touchpad code
            {
                UIntVector  touchPos = Touch::GetPosition();
                u32         minX = code.Right >> 24;
                u32         maxX = (code.Right >> 16) & 0xFF;
                u32         minY = (code.Right >> 8) & 0xFF;
                u32         maxY = code.Right & 0xFF;

                if (!Touch::IsDown()
                    || touchPos.x < minX || touchPos.x > maxX
                    || touchPos.y < minY || touchPos.y > maxY)
                {
                    waitForExitCode = true;
                    conditionCount++;
                }
                break;
            }
            case 0xDF: ///< Registers operations
            {
                u16     operation = code.Right >> 16;
                u16     parameter = code.Right & 0xFFFF;

                if (code.Left == 0x00FFFFFE) ///< Set current data register as vfp
                {
                    currentData.isVFP = code.Right & 1;
                    if (code.Right & 0x10)
                    {
                        if (currentData.isVFP)
                        {
                            const float fvalue = static_cast<float>(currentData.value);
                            currentData.vfp = fvalue;
                        }
                        else
                        {
                            const u32 ivalue = static_cast<u32>(currentData.vfp);
                            currentData.value = ivalue;
                        }
                    }
                    break;
                }
                if (code.Left == 0x00FFFFFF) ///< Conditional mode switch
                {
                    conditionalMode = static_cast<bool>(parameter);
                    break;
                }

                switch (operation)
                {
                case 0x0: ///< Register selection
                {
                    if (code.Left == 0) ///< Offset
                        ActiveOffset = parameter;
                    else ///< Data
                        ActiveData = parameter;
                    break;
                }
                case 0x1: ///< Register copy
                {
                    if (code.Left == 0) ///< Offset
                        Offset[!parameter] = Offset[parameter];
                    else if (code.Left == 1) ///< Data
                        Data[!parameter].value = Data[parameter].value;
                    else if (code.Left == 2) ///< Storage
                        Data[parameter].value = Storage[parameter];
                    break;
                }
                case 0x2:
                {
                    if (code.Left == 0) ///< Offset
                        Data[parameter].value = Offset[parameter];
                    else if (code.Left == 1) ///< Data
                        Offset[parameter] = Data[parameter].value;
                    else if (code.Left == 2) ///< Storage
                        Storage[parameter] = Data[parameter].value;
                    break;
                }
                default:
                    break;
                }
                break;
            }
            case 0xE0: ///< Patch code
            {
                if (code.Data == nullptr || !code.Right)
                    continue; ///< An error occured

                ExitCodeImmediately = !Memcpy(code.Left + Offset[ActiveOffset], code.Data, code.Right);
                break;
            }
            case 0xF1: ///< ADD code
            {
                if (Read32(code.Left + Offset[ActiveOffset], value))
                    ExitCodeImmediately = !Write32(code.Right + Offset[ActiveOffset], value + code.Left);
                break;
            }
            case 0xF2: ///< MUL code
            {
                if (Read32(code.Left + Offset[ActiveOffset], value))
                    ExitCodeImmediately = !Write32(code.Right + Offset[ActiveOffset], value * code.Left);
                break;
            }
            case 0xF3: ///< DIV code
            {
                if (code.Left != 0 && Read32(code.Left + Offset[ActiveOffset], value))
                    ExitCodeImmediately = !Write32(code.Right + Offset[ActiveOffset], value / code.Left);
                break;
            }
            case 0xF4: ///< MUL code on Data[ActiveData]
            {
                value = code.Right;
                if (currentData.isVFP)
                    currentData.vfp *= vfpval;
                else
                    currentData.value *= value;
                break;
            }
            case 0xF5: ///< DIV code on Data[ActiveData]
            {
                value = code.Right;
                if (code.Right != 0)
                {
                    if (currentData.isVFP)
                        currentData.vfp /= vfpval;
                    else
                        currentData.value /= value;
                }
                break;
            }
            case 0xF6: ///< AND code
            {
                currentData.value &= code.Right;
                break;
            }
            case 0xF7: ///< OR code
            {
                currentData.value |= code.Right;
                break;
            }
            case 0xF8: ///< XOR code
            {
                currentData.value ^= code.Right;
                break;
            }
            case 0xF9: ///< NOT code
            {
                currentData.value = ~currentData.value;
                break;
            }
            case 0xFA: ///< Left shift
            {
                currentData.value <<= code.Right;
                break;
            }
            case 0xFB: ///< Right shift
            {
                currentData.value >>= code.Right;
                break;
            }
            case 0xFC: ///< Memcpy code
            {
                if (Offset[0] && Offset[1])
                {
                    ExitCodeImmediately = !Process::CopyMemory((void *)Offset[0], (void *)Offset[1], code.Right);
                }
                break;
            }
            case 0xFF: ///< Random number
            {
                currentData.value = Utils::Random(Data[0].value, Data[0].value);
                break;
            }
            default:
                break;
            }
        }
    error:
        return;
    }
}

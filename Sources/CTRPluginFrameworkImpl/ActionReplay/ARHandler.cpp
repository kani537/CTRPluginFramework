#include "CTRPluginFrameworkImpl/ActionReplay/ARHandler.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFramework/System/Controller.hpp"
#include "CTRPluginFramework/System/Vector.hpp"
#include "CTRPluginFramework/System/Touch.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"

namespace CTRPluginFramework
{
    u32     ARHandler::Offset[2] = { 0 };
    u32     ARHandler::Data[2] = { 0 };
    u32     ARHandler::Storage[2] = { 0 };
    u32     ARHandler::ActiveOffset = 0;
    u32     ARHandler::ActiveData = 0;
    bool    ARHandler::ExitCodeImmediately = false;

    void ARHandler::Execute(const ARCodeVector& arcodes, u32(&storage)[2])
    {
        Offset[0] = 0;
        Offset[1] = 0;
        Data[0] = 0;
        Data[1] = 0;
        Storage[0] = storage[0];
        Storage[1] = storage[1];
        ActiveOffset = 0;
        ActiveData = 0;
        ExitCodeImmediately = false;
        _Execute(arcodes);
        storage[0] = Storage[0];
        storage[1] = Storage[1];
    }


    void    ARHandler::_Execute(const ARCodeVector &codes)
    {
        u32             value = 0;
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
            // If we must exit
            if (ExitCodeImmediately)
                return;

            // If we're waiting for the end of a block
            if (waitForExitCode)
            {
                if (code.Type == 0xD2) ///< Full Terminator
                {
                    conditionCount = 0;
                    Offset[ActiveOffset] = 0;
                    Data[ActiveData] = 0;
                    waitForExitCode = false;
                }
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
                Process::Write32(code.Left + Offset[ActiveOffset], code.Right);
                break;
            }
            case 0x10: ///< Write16
            {
                Process::Write16(code.Left + Offset[ActiveOffset], code.Right);
                break;
            }
            case 0x20: ///< Write8
            {
                Process::Write8(code.Left + Offset[ActiveOffset], code.Right);
                break;
            }
            case 0x30: ///< GreaterThan 32Bits
            {
                if (Process::Read32(code.Left + Offset[ActiveOffset], value)
                    && (conditionalMode ? Data[ActiveData] : code.Right) > value)
                    continue;
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x40: ///< LesserThan 32Bits
            {
                if (Process::Read32(code.Left + Offset[ActiveOffset], value)
                    && (conditionalMode ? Data[ActiveData] : code.Right) < value)
                    continue;
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x50: ///< EqualTo 32Bits
            {
                if (Process::Read32(code.Left + Offset[ActiveOffset], value)
                    && (conditionalMode ? Data[ActiveData] : code.Right) == value)
                    continue;
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x60: ///< NotEqualTo 32Bits
            {
                if (Process::Read32(code.Left + Offset[ActiveOffset], value)
                    && (conditionalMode ? Data[ActiveData] : code.Right) != value)
                    continue;
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x70: ///< GreaterThan 16Bits
            {
                mask = code.Right >> 16;

                if (Process::Read16(code.Left + Offset[ActiveOffset], value16)
                    && (conditionalMode ? Data[ActiveData] & 0xFFFF : code.Right & 0xFFFF) > ~mask & value16)
                    continue;
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x80: ///< LesserThan 16Bits
            {
                mask = code.Right >> 16;
                if (Process::Read16(code.Left + Offset[ActiveOffset], value16)
                    && (conditionalMode ? Data[ActiveData] & 0xFFFF : code.Right & 0xFFFF) < ~mask & value16)
                    continue;
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0x90: ///< EqualTo 16Bits
            {
                mask = code.Right >> 16;
                if (Process::Read16(code.Left + Offset[ActiveOffset], value16)
                    && (conditionalMode ? Data[ActiveData] & 0xFFFF : code.Right & 0xFFFF) == ~mask & value16)
                    continue;
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0xA0: ///< NotEqualTo 16Bits
            {
                mask = code.Right >> 16;
                if (Process::Read16(code.Left + Offset[ActiveOffset], value16)
                    && (conditionalMode ? Data[ActiveData] & 0xFFFF : code.Right & 0xFFFF) != ~mask & value16)
                    continue;
                conditionCount++;
                waitForExitCode = true;
                break;
            }
            case 0xB0: ///< Read Offset
            {
                if (!Process::Read32(code.Left + Offset[ActiveOffset], Offset[ActiveOffset]))
                    goto error;
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
                loopIteration = Data[0];
                loopCount++;
                waitForEndLoop = true;
                break;
            }
            case 0xC2: ///< Loop code data#2
            {
                loopIteration = Data[1];
                loopCount++;
                waitForEndLoop = true;
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
                Data[ActiveData] = 0;
                break;
            }
            case 0xD4: ///< Add to Data[ActiveData]
            {
                Data[ActiveData] += code.Right;
                break;
            }
            case 0xD5: ///< Set Data[ActiveData]
            {
                Data[code.Left] = code.Right;
                break;
            }
            case 0xD6: ///< Write 32bits Data[ActiveData]
            {
                Process::Write32(code.Right + Offset[ActiveOffset], Data[ActiveData]);
                break;
            }
            case 0xD7: ///< Write 16Bits Data[ActiveData]
            {
                value16 = Data[ActiveData];
                Process::Write16(code.Right + Offset[ActiveOffset], value16);
                break;
            }
            case 0xD8: ///< Write 8Bits Data[ActiveData]
            {
                u8 value8 = Data[ActiveData];
                Process::Write8(code.Right + Offset[ActiveOffset], value8);
                break;
            }
            case 0xD9: ///< Read 32bits to Data[ActiveData]
            {
                Process::Read32(code.Right + Offset[ActiveOffset], Data[ActiveData]);
                break;
            }
            case 0xDA: ///< Read 16Bits to Data[ActiveData]
            {
                Process::Read16(code.Right + Offset[ActiveOffset], value16);
                Data[ActiveData] = value16;
                break;
            }
            case 0xDB: ///< Read 8Bits to Data[ActiveData]
            {
                u8 value8 = 0;
                Process::Read8(code.Right + Offset[ActiveOffset], value8);
                Data[ActiveData] = value8;
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
                        Data[!parameter] = Data[parameter];
                    else if (code.Left == 2) ///< Storage
                        Data[parameter] = Storage[parameter];
                    break;
                }
                case 0x2:
                {
                    if (code.Left == 0) ///< Offset
                        Data[parameter] = Offset[parameter];
                    else if (code.Left == 1) ///< Data
                        Offset[parameter] = Data[parameter];
                    else if (code.Left == 2) ///< Storage
                        Storage[parameter] = Data[parameter];
                    break;
                }
                default:
                    break;
                }
                break;
            }
            case 0xE0: ///< Patch code
            {
                if (code.Data == nullptr)
                    continue; ///< An error occured

                Process::CopyMemory(reinterpret_cast<u8 *>(code.Left + Offset[ActiveOffset]), code.Data, code.Right);
                break;
            }
            case 0xF1: ///< ADD code
            {
                if (Process::Read32(code.Left + Offset[ActiveOffset], value))
                    Process::Write32(code.Right + Offset[ActiveOffset], value + code.Left);
                break;
            }
            case 0xF2: ///< MUL code
            {
                if (Process::Read32(code.Left + Offset[ActiveOffset], value))
                    Process::Write32(code.Right + Offset[ActiveOffset], value * code.Left);
                break;
            }
            case 0xF3: ///< DIV code
            {
                if (code.Left != 0 && Process::Read32(code.Left + Offset[ActiveOffset], value))
                    Process::Write32(code.Right + Offset[ActiveOffset], value / code.Left);
                break;
            }
            case 0xF4: ///< MUL code on Data[ActiveData]
            {
                Data[ActiveData] *= code.Right;
                break;
            }
            case 0xF5: ///< DIV code on Data[ActiveData]
            {
                if (code.Right != 0)
                    Data[ActiveData] /= code.Right;
                break;
            }
            case 0xF6: ///< AND code
            {
                Data[ActiveData] &= code.Right;
                break;
            }
            case 0xF7: ///< OR code
            {
                Data[ActiveData] |= code.Right;
                break;
            }
            case 0xF8: ///< XOR code
            {
                Data[ActiveData] ^= code.Right;
                break;
            }
            case 0xF9: ///< NOT code
            {
                Data[ActiveData] = ~Data[ActiveData];
                break;
            }
            case 0xFA: ///< Left shift
            {
                Data[ActiveData] <<= code.Right;
                break;
            }
            case 0xFB: ///< Right shift
            {
                Data[ActiveData] >>= code.Right;
                break;
            }
            case 0xFC: ///< Memcpy code
            {
                if (Offset[0] && Offset[1])
                {
                    Process::CopyMemory((void *)Offset[0], (void *)Offset[1], code.Right);
                }
                break;
            }
            case 0xFF: ///< Random number
            {
                Data[ActiveData] = Utils::Random(Data[0], Data[1]);
                break;
            }
            default:
                continue;
            }
        }
    error:
        return;
    }
}

#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARCodeEditor.hpp"
#include "CTRPluginFramework/Utils.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "CTRPluginFramework/System/System.hpp"
#include "Unicode.h"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

#define PATCH_COLOR Color::Grey
#define TYPE_COLOR Color::Brown
#define IMMEDIATE_COLOR Color::Blue
#define MASK_COLOR Color::Orange
#define OFFSET_COLOR Color::ForestGreen
#define CONTROL_COLOR Color::Red
#define UNUSED_COLOR Color::Black

namespace CTRPluginFramework
{
    static const char       *__emptyCode = "00000000 00000000";
    static ARCodeEditor     *__arCodeEditor = nullptr;

#define IsEmpty (flags & ARCodeEditor::CodeLine::Empty)
#define IsError (flags & ARCodeEditor::CodeLine::Empty)
#define IsModified (flags & ARCodeEditor::CodeLine::Modified)
#define IsData (flags & ARCodeEditor::CodeLine::PatchData)
#define Clear(bit) (flags &= ~bit)
#define Set(bit) (flags |= bit)

    static std::string      ColorToString(const Color &color)
    {
        char  strColor[5] = { 0 };

        strColor[0] = 0x1B;
        strColor[1] = std::max((u8)1, color.r);
        strColor[2] = std::max((u8)1, color.g);
        strColor[3] = std::max((u8)1, color.b);

        return (strColor);
    }

#define ARROW_UP "\xFF"
#define ARROW_DOWN "\x19"
#define ARROW_LEFT "\xFE"
#define ARROW_RIGHT "\x1A"

    static std::string  KeysToString(u32 keys)
    {
        static const char * keysText[] =
        {
            "A", "B", "Select", "Start", ARROW_RIGHT, ARROW_LEFT, ARROW_UP, ARROW_DOWN,
            "R", "L", "X", "Y", "", "", "ZL", "ZR", "", "", "", "", "Touch", "", "", "",
            "SR", "SL", "SU", "SD", "CR", "CL", "CU", "CD"
        };

        std::string ret;
        bool plus = false;

        for (u32 i = 0; i < 32; ++i)
        {
            if (keys & (1u << i))
            {
                std::string key = keysText[i];
                if (key.empty())
                    continue;
                if (plus)
                    ret += '+';
                ret += key;
                plus = true;
            }
        }

        return ret;
    }

    static std::string      ColorCodeLine(const ARCode &code)
    {
        std::string ret = code.Text;

        switch (code.Type)
        {
        case 0x00: ///< Write32
        case 0x30: ///< GT 32Bits
        case 0x40: ///< LT 32Bits
        case 0x50: ///< EQ 32Bits
        case 0x60: ///< NE 32Bits
        case 0xE0: ///< Patch code
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(5 /* 1 + Color */, ColorToString(OFFSET_COLOR));
            ret.insert(17 /* 9 + Color * 2 */, ColorToString(IMMEDIATE_COLOR));
            break;
        case 0x10: ///< Write16
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(5 /* 1 + Color */, ColorToString(OFFSET_COLOR));
            ret.insert(17 /* 9 + Color * 2 */, ColorToString(UNUSED_COLOR));
            ret.insert(25 /* 13 + Color * 3 */, ColorToString(IMMEDIATE_COLOR));
            break;
        case 0x20: ///< Write8
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(5 /* 1 + Color */, ColorToString(OFFSET_COLOR));
            ret.insert(17 /* 9 + Color * 2 */, ColorToString(UNUSED_COLOR));
            ret.insert(27 /* 15 + Color * 3 */, ColorToString(IMMEDIATE_COLOR));
            break;

        case 0x70: ///< GT 16Bits
        case 0x80: ///< LT 16Bits
        case 0x90: ///< EQ 16Bits
        case 0xA0: ///< NE 16Bits
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(5 /* 1 + Color */, ColorToString(OFFSET_COLOR));
            ret.insert(17 /* 9 + Color * 2 */, ColorToString(MASK_COLOR));
            ret.insert(25 /* 13 + Color * 3 */, ColorToString(IMMEDIATE_COLOR));
            break;

        case 0xB0: ///< Read & Set Offset
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(5 /* 1 + Color */, ColorToString(OFFSET_COLOR));
            ret.insert(17 /* 9 + Color * 2 */, ColorToString(UNUSED_COLOR));
            break;

        case 0xC0: ///< Set loop count
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(5 /* 1 + Color */, ColorToString(UNUSED_COLOR));
            ret.insert(17 /* 9 + Color * 2 */, ColorToString(IMMEDIATE_COLOR));
            break;

        case 0xC1:
        case 0xC2:
        case 0xD3: ///< Set offset
        case 0xDC: ///< Add to offset
        case 0xD4: ///< Add to register
        case 0xD6: ///< Set data
        case 0xD7:
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDD: ///< Check input
        case 0xF4: ///< MUL
        case 0xF5: ///< DIV
        case 0xF6: ///< AND
        case 0xF7: ///< OR
        case 0xF8: ///< XOR
        case 0xFA: ///< LSL
        case 0xFB: ///< LSR
        case 0xFC: ///< Copy
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(6 /* 2 + Color */, ColorToString(UNUSED_COLOR));
            ret.insert(17 /* 9 + Color * 2 */, ColorToString(IMMEDIATE_COLOR));
            break;

        case 0xD2: ///< Full terminator
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(6 /* 2 + Color */, ColorToString(UNUSED_COLOR));
            if (code.Right)
                ret.insert(24 /* 16 + Color * 2 */, ColorToString(IMMEDIATE_COLOR));
            break;

        case 0xD1: ///< Loop execute
        case 0xD0: ///< Terminator
        case 0xF9: ///< NOT operation
        case 0xFF: ///< Random
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(6 /* 2 + Color */, ColorToString(UNUSED_COLOR));
            break;

        case 0xD5: ///< Set data register
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(6 /* 2 + Color */, ColorToString(UNUSED_COLOR));

            if (code.Left == 0)
                ret.insert(17 /* 9 + Color * 2 */, ColorToString(IMMEDIATE_COLOR));
            else
            {
                ret.insert(15 /* 7 + Color * 2 */, ColorToString(Color::Red));
                ret.insert(21 /* 9 + Color * 3 */, ColorToString(IMMEDIATE_COLOR));
            }
            break;

        case 0xDE: ///< Check touchpad:
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(6 /* 2 + Color */, ColorToString(UNUSED_COLOR));
            ret.insert(15 /* 7 + Color * 2 */, ColorToString(Color::Red));
            ret.insert(21 /* 9 + Color * 3 */, ColorToString(IMMEDIATE_COLOR));
            ret.insert(29 /* 13 + Color * 4 */, ColorToString(Color::Green));
            break;

        case 0xDF: ///< Register
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(6 /* 2 + Color */, ColorToString(Color::Grey));
            ret.insert(17 /* 9 + Color * 2 */, ColorToString(MASK_COLOR));
            ret.insert(25 /* 13 + Color * 3 */, ColorToString(IMMEDIATE_COLOR));
            break;

        case 0xF1: ///< Custom Codes
        case 0xF2:
        case 0xF3:
            ret.insert(0, ColorToString(TYPE_COLOR));
            ret.insert(6 /* 2 + Color */, ColorToString(OFFSET_COLOR));
            ret.insert(17 /* 9 + Color * 2 */, ColorToString(IMMEDIATE_COLOR));
            break;

        default:
            ret.insert(0, ColorToString(Color::Red));
            break;
        }
        return (ret);
    }

    static std::string      CommentCodeLine(const ARCode &code)
    {
        u32         mask;
        u32         value;
        const char  *reg;
        std::string ret;

        switch (code.Type)
        {
        case 0x00: ///< Write32
            ret = Utils::Format("[%08X + offs] = %08X", code.Left, code.Right);
            break;
        case 0x10: ///< Write16
            ret = Utils::Format("[%08X + offs] = %04X", code.Left, (code.Right & 0xFFFF));
            break;
        case 0x20: ///< Write8
            ret = Utils::Format("[%08X + offs] = %02X", code.Left, (code.Right & 0xFF));
            break;

        case 0x30: ///< GT 32Bits
            ret = Utils::Format("if %08X > [%08X+offs]:", code.Right, code.Left);
            break;
        case 0x40: ///< LT 32Bits
            ret = Utils::Format("if %08X < [%08X+offs]:", code.Right, code.Left);
            break;
        case 0x50: ///< EQ 32Bits
            ret = Utils::Format("if %08X == [%08X+off]:", code.Right, code.Left);
            break;
        case 0x60: ///< NE 32Bits
            ret = Utils::Format("if %08X != [%08X+off]:", code.Right, code.Left);
            break;

        case 0x70: ///< GT 16Bits
            mask = code.Right >> 16;
            value = code.Right & 0xFFFF;
            if (mask)
                ret = Utils::Format("if %04X>[%08X+off] & %04X:", value, code.Left, (~mask & 0xFFFF));
            else
                ret = Utils::Format("if %04X > [%08X+offs]:", value, code.Left);
            break;
        case 0x80: ///< LT 16Bits
            mask = code.Right >> 16;
            value = code.Right & 0xFFFF;
            if (mask)
                ret = Utils::Format("if %04X<[%08X+off] & %04X:", value, code.Left, (~mask & 0xFFFF));
            else
                ret = Utils::Format("if %04X < [%08X+offs]:", value, code.Left);
            break;
        case 0x90: ///< EQ 16Bits
            mask = code.Right >> 16;
            value = code.Right & 0xFFFF;
            if (mask)
                ret = Utils::Format("if %04X==[%08X+of] & %04X:", value, code.Left, (~mask & 0xFFFF));
            else
                ret = Utils::Format("if %04X == [%08X+offs]:", value, code.Left);
            break;
        case 0xA0: ///< NE 16Bits
            mask = code.Right >> 16;
            value = code.Right & 0xFFFF;
            if (mask)
                ret = Utils::Format("if %04X!=[%08X+of] & %04X:", value, code.Left, (~mask & 0xFFFF));
            else
                ret = Utils::Format("if %04X != [%08X + offset]:", value, code.Left);
            break;

        case 0xB0: ///< Read and Set Offset
            ret = Utils::Format("offset = [%08X + offset]", code.Left);
            break;
        case 0xD3: ///< Set offset to immediate
            ret = Utils::Format("offset = %08X", code.Right);
            break;
        case 0xDC: ///< Add to offset
            ret = Utils::Format("offset += %08X", code.Right);
            break;
        case 0xC0: ///< Loop
            ret = Utils::Format("Loop %X times:", code.Right);
            break;
        case 0xC1: ///< Loop
            ret = "loop [data#1] times";
            break;
        case 0xC2: ///< Loop
            ret = "loop [data#2] times";
            break;

        case 0xD2: ///< Full terminator
            if (code.Right & 1)
                ret = "exit code immediately";
            else
                ret = "end all if,start loop,clr reg";
            break;

        case 0xD1: ///< Loop execute
            ret = "start loop";
            break;
        case 0xD0: ///< Terminator
            ret = "end if";
            break;

        case 0xD4: ///< Add to register
            ret = Utils::Format("data += %08X", code.Right);
            break;
        case 0xD5: ///< Set register
            reg = code.Left & 1 ? "Data#2" : "Data#1";
            ret = Utils::Format("%s = %08X", reg, code.Right);
            break;
        case 0xD6: ///< Write32
            ret = Utils::Format("[%08X+off] = data, off+=4", code.Right);
            break;
        case 0xD7: ///< Write16
            ret = Utils::Format("[%08X+off] = data, off+=2", code.Right);
            break;
        case 0xD8: ///< Write8
            ret = Utils::Format("[%08X+off] = data, off+=1", code.Right);
            break;

        case 0xD9: ///< Set data 32
            ret = Utils::Format("data = [%08X+offs]", code.Right);
            break;
        case 0xDA: ///< Set data 16
            ret = Utils::Format("data = [%08X+offs] & FFFF", code.Right);
            break;
        case 0xDB: ///< Set data 8
            ret = Utils::Format("data = [%08X+offs] & FF", code.Right);
            break;

        case 0xE0: ///< E code
            if (code.Right)
                ret = Utils::Format("[%08X+offs] = patch data", code.Left);
            else
                ret = "!! error !!";
            break;

        case 0xDD: ///< Check input
            ret = "if ";
            ret += KeysToString(code.Right);
            ret += ":";
            break;

        case 0xDE: ///< Check touch:
        {
            if (code.Left == 0)
            {
                u32 minX = code.Right >> 16;
                u32 maxX = (code.Right << 16) >> 16;
                ret = Utils::Format("if %04X <= touch.X <= %04X:", minX, maxX);
            }
            else
            {
                u32 minY = code.Right >> 16;
                u32 maxY = (code.Right << 16) >> 16;
                ret = Utils::Format("if %04X <= touch.Y <= %04X:", minY, maxY);
            }
            break;
        }
        case 0xDF: ///< Registers:
        {
            // VFP Toggle
            if (code.Left == 0xFFFFFFE)
            {
                bool conversion = code.Right & 0x10 > 0;
                reg = code.Right & 1 > 0 ? "enabled" : "disabled";
                ret = Utils::Format("data vfp state %s", reg);
                if (conversion)
                    ret += ", cvt";
                break;
            }
            // Condition toggle
            if (code.Left == 0xFFFFFF)
            {
                ret = code.Right > 0 ? "cond. compared to immediate" : "cond. compared to data";
                break;
            }

            u32 operation = code.Right >> 16;
            u32 control = code.Right & 0xFFFF;

            static const char *offsets[2] = {"offset#1", "offset#2"};
            static const char *data[2] = {"data#1", "data#2"};
            static const char *storage[2] = {"storage#1", "storage#2"};

            const char *from = nullptr;
            const char *to = nullptr;
            switch (operation)
            {
            case 0: ///< Toggle active register
            {
                if (code.Left == 0) ///< change active offset
                    reg =  offsets[control > 0];
                else if (code.Left == 1) ///> change active data
                    reg =  data[control > 0];
                else break;
                ret = Utils::Format("Set %s as active", reg);
                break;
            }
            case 1: ///< Copy to other index
            {
                if (code.Left == 0)
                {
                    from = offsets[control > 0];
                    to = offsets[control == 0];
                }
                else if (code.Left == 1)
                {
                    from = data[control > 0];
                    to = data[control == 0];
                }
                else if (code.Left == 2)
                {
                    from = storage[control > 0];
                    to = data[control > 0];
                }

                break;
            }
            case 2: ///< Copy to other register
            {
                if (code.Left == 0)
                {
                    from = offsets[control > 0];
                    to = data[control > 0];
                }
                else if (code.Left == 1)
                {
                    from = data[control > 0];
                    to = offsets[control > 0];
                }
                else if (code.Left == 2)
                {
                    from = data[control > 0];
                    to = storage[control > 0];
                }

                break;
            }
            default:
                break;
            }
            if (to && from)
                ret = Utils::Format("%s = %s", to, from);
            break;
        }

        case 0xF1: ///< Add to addr
            ret = Utils::Format("[%08X+offs] += %08X", code.Left, code.Right);
            break;
        case 0xF2: ///< Mul to addr
            ret = Utils::Format("[%08X+offs] *= %08X", code.Left, code.Right);
            break;
        case 0xF3: ///< Div to addr
            ret = Utils::Format("[%08X+offs] /= %08X", code.Left, code.Right);
            break;
        case 0xF4: /// Mul data
            ret = Utils::Format("data *= %08X", code.Right);
            break;
        case 0xF5: ///< Div data
            ret = Utils::Format("data /= %08X", code.Right);
            break;
        case 0xF6: ///< AND data
            ret = Utils::Format("data &= %08X", code.Right);
            break;
        case 0xF7: ///< OR data
            ret = Utils::Format("data |= %08X", code.Right);
            break;
        case 0xF8: ///< XOR data
            ret = Utils::Format("data ^= %08X", code.Right);
            break;
        case 0xF9: ///< NOT data
            ret = "data = ~data";
            break;
        case 0xFA: ///< LSL data
            ret = Utils::Format("data <<= %08X", code.Right);
            break;
        case 0xFB: ///< LSR data
            ret = Utils::Format("data >>= %08X", code.Right);
            break;
        case 0xFC: ///< Copy
            ret = Utils::Format("Copy %08X, off#2 => off#1", code.Right);
            break;
        default:
            ret = "!! error !!";
            break;
        }

        if (ret.size() > 29)
            ret.erase(30);
        return (ret);
    }

    ARCodeEditor::CodeLine::CodeLine(ARCode &code) :
        base(code), parent(nullptr), flags(0), index(0)
    {
        if (code.Text.empty())
            code.Text = code.ToString();
        Set(Modified);
    }

    ARCodeEditor::CodeLine::CodeLine(const CodeLine &right) :
        base(right.base)
    {
        parent = right.parent;
        flags = right.flags;
        index = right.index;
        Set(Modified);
    }

    ARCodeEditor::CodeLine::CodeLine(CodeLine &&right) :
        base(right.base)
    {
        parent = right.parent;
        flags = right.flags;
        index = right.index;
        Set(Modified);
    }

    #define IsInRange(val, min, max) (val >= min && val <= max)

    ARCodeEditor::CodeLine &ARCodeEditor::CodeLine::operator=(CodeLine &&right)
    {
        base = std::move(right.base);
        parent = right.parent;
        flags = right.flags;
        index = right.index;
        Set(Modified);
        return *this;
    }

    void    ARCodeEditor::CodeLine::Edit(u32 cursor, u32 value)
    {
        if (IsData)
        {
            if (!parent)
                return;

            u32 &dst = cursor >= 8 ? parent->Data[index + 1] : parent->Data[index];

            if (cursor >= 8)
                cursor -= 9;

            u32 shift = (7 - cursor) * 4;
            value <<= shift;
            u32 mask = ~(0xF << shift);
            dst &= mask;
            dst |= value;
        }
        else
        {
            value &= 0xFF;
            if (IsInRange(value, 0, 9))
                value += '0';
            else if (IsInRange(value, 10, 15))
                value = (value - 10) + 'A';
            else
                return;

            // If the codetype change for E
            if (cursor == 0 && value == 'E' && base.Text[cursor] != 'E')
            {
                // Be sure that the whole line is empty
                base.Text = __emptyCode;
            }

            // If what changes is the size to patch for E code
            else if (cursor >= 8 && base.Text[0] == 'E')
            {
                bool            error;
                int             currentLines = base.Right / 8 + (base.Right % 8 > 0 ? 1 : 0);
                int             newLines = 0;
                int             diff = 0;
                std::string     right = base.Text.substr(9, 8);

                right[(cursor > 8 ? cursor - 9 : cursor)] = value;

                newLines = ActionReplayPriv::Str2U32(right, error);
                if (!error)
                    newLines = newLines / 8 + (newLines % 8 > 0 ? 1 : 0);
                else
                    newLines = 0;

                diff = currentLines - newLines;

                if (diff > 0)
                {
                    std::string body = Utils::Format("You're about to delete %d line(s) of data, continue ?", diff);
                    if (!MessageBox(Color::Orange << "Warning", body, DialogType::DialogYesNo)())
                        return;
                }

                base.Text[cursor] = value;
                if (!base.Update(base.Text))
                {
                    if (base.Right > 0)
                        base.Data.resize((base.Right / 8 + ((base.Right % 8) > 0 ? 1 : 0)) * 2);
                    else
                        base.Data.clear();
                    __arCodeEditor->_ReloadCodeLines();
                    return;
                }
            }
            // If the code type changes from E to something else, ask before pursue
            else if (cursor == 0 && base.Text[0] == 'E' && value != 'E' && base.Right > 0)
            {
                if (!MessageBox(Color::Orange << "Warning", "You're about to delete the E code with all it's data, continue ?", DialogType::DialogYesNo)())
                    return;
                // Be sure that the whole line is empty
                base.Text = __emptyCode;
                base.Data.clear();
                base.Update(base.Text);
                base.Text[0] = value;
                __arCodeEditor->_ReloadCodeLines();
                return;
            }
            base.Text[cursor] = value;
        }

        Set(Modified);
    }

    void    ARCodeEditor::CodeLine::Update(void)
    {
        // If current line is E code data
        if (IsData && IsModified)
        {
            /*
            std::string lStr = base.Text.substr(0, 8);
            std::string rStr = base.Text.substr(9, 8);

            bool error = false;

            u32 left = ActionReplayPriv::Str2U32(lStr, error);
            if (error) left = 0;
            u32 right = ActionReplayPriv::Str2U32(rStr, error);
            if (error) right = 0;

              if (parent)
            {
                parent->Data[index] = left;
                parent->Data[index + 1] = right;
            }
            */
            display = ColorToString(PATCH_COLOR) + Utils::Format("%08X %08X", parent->Data[index], parent->Data[index + 1]);
            comment = "patch data";
            Clear(Modified);
            return;
        }

        if (IsModified)
        {
            bool error = base.Update(base.Text);

            if (error)
            {
                display = Color::Red << base.Text;
                comment = "!! error !!";
            }
            else
            {
                display = ColorCodeLine(base);
                comment = CommentCodeLine(base);
            }

            Clear(Modified);
        }
    }

    static void ShowHelp(void)
    {
        std::string body = "Controls:\n" \
            "    - " FONT_B ": Exit editor (changes are applied directly)\n" \
            "    - " FONT_Y ": Delete current code\n" \
            "    - " FONT_L ": Insert a new code before current code\n" \
            "    - " FONT_R ": Insert a new code after current code\n";
        if (System::IsNew3DS())
            body += "    - " FONT_ZL ": Copy current code to clipboard\n" \
            "    - " FONT_ZR ": Clear clipboard\n";
        body += "    - \uE006: Navigate in the code";

        MessageBox(Color::LimeGreen << "Action Replay Code Editor Help",  body)();

        ScreenImpl::Clean();

        float fade = 0.03f;
        Clock t = Clock();
        Time limit = Seconds(1) / 10.f;
        Time delta;
        float pitch = 0.0006f;

        while (fade <= 0.3f)
        {
            delta = t.Restart();
            fade += pitch * delta.AsMilliseconds();

            ScreenImpl::Top->Fade(fade);
            ScreenImpl::Bottom->Fade(fade);

            ScreenImpl::Top->SwapBuffer(true, true);
            ScreenImpl::Bottom->SwapBuffer(true, true);
            gspWaitForVBlank();
            if (System::IsNew3DS())
                while (t.GetElapsedTime() < limit);
        }
    }

    /*
     * Editor
     */
    ARCodeEditor::ARCodeEditor(void) :
        _submenu{ { "Copy to clipboard", "Clear clipboard", "Delete all codes", "Converter", "Help" } }
    {
        _exit = false;
        _index =  _line = 0;
        _keyboard.SetLayout(Layout::HEXADECIMAL);
        _keyboard._Hexadecimal();
        _keyboard._showCursor = false;
        __arCodeEditor = this;
        _clipboard = nullptr;
        _context = nullptr;
    }

    bool    ARCodeEditor::operator()(EventList &eventList)
    {
        // Process event
        bool isSubMenuOpen = _submenu.IsOpen();
        for (Event &event : eventList)
        {
            _submenu.ProcessEvent(event);
            if (!isSubMenuOpen)
                _ProcessEvent(event);
        }

        int out; ///< Used for keyboard's input

        if (_submenu.IsOpen())
        {
            switch (_submenu())
            {
            case 0: ///< Copy to clipboard
            {
                if (_codes.empty() || !_context)
                    break;

                if (_clipboard)
                    delete _clipboard;
                _clipboard = new ARCode(_context->codes[_line]);
                break;
            }
            case 1: ///< Clear clipboard
            {
                if (_clipboard)
                {
                    delete _clipboard;
                    _clipboard = nullptr;
                }
                break;
            }
            case 2: ///< Clear all codes
            {
                if (!(MessageBox(Color::Orange << "Warning", "Do you really want to delete all codes ?", DialogType::DialogYesNo)()))
                    break;

                if (_context)
                    _context->codes.clear();

                _ReloadCodeLines();

                break;
            }
            case 3: ///< Converter
                _converter();
                break;
            case 4: ///< Show help
                ShowHelp();
                break;
            default:
                break;
            }
        }
        else if (_keyboard(out) && !_codes.empty())
        {
            _codes[_line].Edit(_index, out);
            if (_index < 16)
                _index++;
            if (_index == 8) _index++;
        }

        // Update states
        _Update();

        // Draw top screen
        _RenderTop();

        // Draw bottom screen
        _RenderBottom();

        return _exit;
    }

    void    ARCodeEditor::Edit(ARCodeContext &ctx)
    {
        if (!__arCodeEditor)
            return;

        ARCodeEditor &editor = *__arCodeEditor;

        editor._context = &ctx;
        editor._exit = false;
        editor._index = editor._line = 0;
        editor._codes.clear();

        editor._ReloadCodeLines();

        Event           event;
        EventList       eventList;
        EventManager    manager;
        bool            exit = false;

        do
        {
            eventList.clear();
            // Fetch all events
            while (manager.PollEvent(event))
                eventList.push_back(event);

            // Execute Editor's loop
            exit = editor(eventList);

            // Swap screens
            Renderer::EndFrame();

        } while (!exit);
    }

    void    ARCodeEditor::_ProcessEvent(Event &event)
    {
        if (event.type == Event::KeyPressed)
        {
            switch (event.key.code)
            {
            case Key::B:
            {
                _exit = true;
                break;
            }
            // Insert Before
            case Key::L:
            {
                bool error;
                ARCodeVector &codes = _context->codes;

                // If code is empty, simply push something
                if (_codes.empty())
                {
                    _line = 0;
                    if (!_clipboard)
                        codes.push_back(ARCode(__emptyCode, error));
                    else
                        codes.push_back(ARCode(*_clipboard));
                    _ReloadCodeLines();
                    break;
                }

                // If we're in the middle of E's data, don't insert anything
                if (_codes[_line].flags & CodeLine::PatchData)
                    break;

                if (!_clipboard)
                    codes.insert(codes.begin() + _line, ARCode(__emptyCode, error));
                else
                    codes.insert(codes.begin() + _line, ARCode(*_clipboard));
                _ReloadCodeLines();
                break;
            }
            // Insert After
            case Key::R:
            {
                bool error;
                ARCodeVector &codes = _context->codes;

                // If code is empty or we'reat the end, simply push something
                if (_codes.empty() || _line == _codes.size() - 1)
                {
                    if (!_clipboard)
                        codes.push_back(ARCode(__emptyCode, error));
                    else
                        codes.push_back(ARCode(*_clipboard));
                    _ReloadCodeLines();
                    break;
                }

                int line = _line + 1;

                // If we're in the middle of E's data, don't insert anything
                if (_codes[line].flags & CodeLine::PatchData)
                    break;

                if (!_clipboard)
                    codes.insert(codes.begin() + line, ARCode(__emptyCode, error));
                else
                    codes.insert(codes.begin() + line, ARCode(*_clipboard));
                _ReloadCodeLines();
                break;
            }
            // Copy code to clipboard
            case Key::ZL:
            {
                if (_codes.empty() || !_context)
                    break;

                if (_clipboard)
                    delete _clipboard;
                _clipboard = new ARCode(_context->codes[_line]);
                break;
            }
            // Clear clipboard
            case Key::ZR:
            {
                if (_clipboard)
                {
                    delete _clipboard;
                    _clipboard = nullptr;
                }
                break;
            }
            // Delete code
            case Key::Y:
            {
                if (_line >= _codes.size()) break;

                // If we're in the middle of E's data, don't remove anything
                if (_codes[_line].flags & CodeLine::PatchData)
                    break;

                if (!(MessageBox(Color::Orange << "Warning", "Do you really want to delete this code ?", DialogType::DialogYesNo))())
                    break;

                _context->codes.erase(_context->codes.begin() + _line);
                _ReloadCodeLines();
                break;
            }
            default:
                break;
            }///< End switch event.key.code
        }///< End if Event::KeyPressed

        if (event.type == Event::KeyDown && _inputClock.HasTimePassed(Milliseconds(150)))
        {
            switch (event.key.code)
            {
            case Key::DPadUp:
                _line = std::max((int)0, (int)_line - 1);
                break;

            case Key::DPadDown:
                _line = std::min(_line + 1, (int)(_codes.size() - 1));
                break;

            case Key::DPadLeft:
                _index = std::max(_index - 1, 0);
                if (_index == 8)
                    _index--;
                break;

            case Key::DPadRight:
                _index = std::min(_index + 1, 16);
                if (_index == 8)
                    _index++;
                break;
            default:
                break;
            }///< End switch event.key.code
            _inputClock.Restart();
        }///< End if Event::KeyHold
    }

    void    ARCodeEditor::_RenderTop(void)
    {
        Renderer::SetTarget(TOP);
        Window::TopWindow.Draw("Code Editor");

        // Column headers

        // First column: 31px : 3*7 + 10 => Line
        // Second column: 112px : (17* 6) + 10 => Code
        // Last column: 185px : 175 + 10 => Comment
        // Total: 312px :  31 + 1 + 112 + 1 + 185
        // Margin left / right: 5px

        int   posY = 61;
        // Line header
        Renderer::DrawRect(35, posY, 31, 20, Color::SkyBlue);

        // Code header
        Renderer::DrawRect(67, posY, 112, 20, Color::DeepSkyBlue);

        // Comment header
        Renderer::DrawRect(180, posY, 185, 20, Color::DeepSkyBlue);

        posY += 21;

        // Line body
        Renderer::DrawRect(35, posY, 31, 112, Color::DeepSkyBlue);
        // Code body
        Renderer::DrawRect(67, posY, 112, 112, Color::Blank);
        // Comment body
        Renderer::DrawRect(180, posY, 185, 112, Color::SkyBlue);

        // If there's no code, exit here
        if (_codes.empty())
        {
            _submenu.Draw();
            return;
        }

        // Draw cursor
        Renderer::DrawRect(_cursorPosX, _cursorPosY, 7, 10, Color::SkyBlue);

        // Draw codes
        int posX = 72;
        int posXline = 40;
        int posXComment = 185;
        posY += 2;
        int posYline = posY;
        int posYComment = posY;
        {
            int i = std::max(0, _line - 10);
            int max = std::min(i + 11, (int)_codes.size());

            for (; i < max; ++i)
            {
                // Draw line
                Renderer::DrawString(Utils::Format("%3d", i + 1).c_str(), posXline, posYline, Color::Black);
                // Draw code
                Renderer::DrawString(_codes[i].display.c_str(), posX, posY, Color::Black);
                // Draw comment
                Renderer::DrawString(_codes[i].comment.c_str(), posXComment, posYComment, Color::DimGrey);
            }
        }

        const Color    &textcolor = Preferences::Settings.MainTextColor;
        posY = 203;
        Renderer::DrawString((char *)"Options:", 260, posY, textcolor);
        posY -= 14;
        Renderer::DrawSysString((char *)"\uE002", 320, posY, 380, textcolor);
        _submenu.Draw();
    }

    void    ARCodeEditor::_RenderBottom(void)
    {
        Renderer::SetTarget(BOTTOM);
        _keyboard._RenderBottom();
    }

    void    ARCodeEditor::_Update(void)
    {
        if (_submenu.IsOpen())
            return;

        // Update cursor pos
        _cursorPosX = 71 + _index * 6;
        int start = std::max(_line - 10, 0);
        _cursorPosY = 83 + (_line - start) * 10;

        for (CodeLine &code : _codes)
            code.Update();
    }

    void    ARCodeEditor::_ReloadCodeLines(void)
    {
        static std::vector<ARCode> tempar;

        _codes.clear();
        tempar.clear();

        if (!_context)
            return;

        ARCodeVector &arcodes = _context->codes;

        for (ARCode &code : arcodes)
        {
            _codes.push_back(CodeLine(code));
            // If the code is E type, add all its data
            if (code.Type == 0xE0 && !code.Data.empty())
            {
                for (int i = 0; i < code.Data.size() - 1; i += 2)
                {
                    bool error;
                    std::string data = Utils::Format("%08X %08X", code.Data[i], code.Data[i + 1]);
                    ARCode temp(data, error);
                    temp.Left = code.Data[i];
                    temp.Right = code.Data[i + 1];
                    tempar.push_back(temp);

                    _codes.push_back(CodeLine(tempar.back()));
                    _codes.back().parent = &code;
                    _codes.back().flags |= CodeLine::PatchData;
                    _codes.back().index = i;
                }
            }
        }

        while (_line >= _codes.size() && _line > 0)
            _line--;
        if (_line < 0)
            _line = 0;
    }
}

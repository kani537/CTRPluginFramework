#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARCodeEditor.hpp"
#include "CTRPluginFramework/Utils.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"

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
            ret.insert(17 /* 9 + Color * 2 */, ColorToString(IMMEDIATE_COLOR));
            ret.insert(23 /* 11 + Color * 3 */, ColorToString(Color::Red));
            ret.insert(29 /* 13 + Color * 4 */, ColorToString(Color::Blue));
            ret.insert(35 /* 15 + Color * 5 */, ColorToString(Color::Green));
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
                base.Text[cursor] = value;
                if (!base.Update(base.Text))
                {
                    base.Data.resize(base.Right / 4 + ((base.Right % 8) > 0 ? 2: 0));
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
            Clear(Modified);
            return;
        }

        if (IsModified)
        {
            bool error = base.Update(base.Text);

            if (error)
                display = Color::Red << base.Text;
            else
                display = ColorCodeLine(base);

            Clear(Modified);
        }
    }

    /*
     * Editor
     */
    ARCodeEditor::ARCodeEditor(void)
    {
        _exit = false;
        _index =  _line = 0;
        _keyboard.SetLayout(Layout::HEXADECIMAL);
        _keyboard._Hexadecimal();
        __arCodeEditor = this;
        _clipboard = nullptr;
        _context = nullptr;
    }

    bool    ARCodeEditor::operator()(EventList &eventList)
    {
        // Process event
        for (Event &event : eventList)
            _ProcessEvent(event);
        int out;

        if (_keyboard(out))
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

    void    ARCodeEditor::_DrawSubMenu(void)
    {
    }

    void    ARCodeEditor::_RenderTop(void)
    {
        Renderer::SetTarget(TOP);
        Window::TopWindow.Draw("Code Editor");

        // Column headers

        // First column: 31px : 3*7 + 10 => Line
        // Second column: 112px : (17* 6) + 10 => Code
        // Last column: 167px : 157 + 10 => Comment
        // Total: 312px :  31 + 1 + 112 + 1 + 167
        // Margin left / right: 14px

        int   posY = 61;
        // Line header
        Renderer::DrawRect(44, posY, 31, 20, Color::SkyBlue);

        // Code header
        Renderer::DrawRect(76, posY, 112, 20, Color::DeepSkyBlue);

        // Comment header
        Renderer::DrawRect(189, posY, 167, 20, Color::DeepSkyBlue);

        posY += 21;

        // Line body
        Renderer::DrawRect(44, posY, 31, 112, Color::DeepSkyBlue);
        // Code body
        Renderer::DrawRect(76, posY, 112, 112, Color::Blank);
        // Comment body
        Renderer::DrawRect(189, posY, 167, 112, Color::SkyBlue);

        // If there's no code, exit here
        if (_codes.empty())
            return;

        // Draw cursor
        Renderer::DrawRect(_cursorPosX, _cursorPosY, 7, 10, Color::SkyBlue);

        // Draw codes
        int posX = 81;
        int posXline = 49;
        posY += 2;
        int posYline = posY;
        {
            int i = std::max(0, _line - 10);
            int max = std::min(i + 11, (int)_codes.size());

            for (; i < max; ++i)
            {
                // Draw line
                Renderer::DrawString(Utils::Format("%3d", i + 1).c_str(), posXline, posYline, Color::Black);
                // Draw code
                Renderer::DrawString(_codes[i].display.c_str(), posX, posY, Color::Black);
            }
        }

        // Draw comments ?
    }

    void    ARCodeEditor::_RenderBottom(void)
    {
        Renderer::SetTarget(BOTTOM);
        _keyboard._RenderBottom();
    }

    void    ARCodeEditor::_Update(void)
    {
        // Update cursor pos
        _cursorPosX = 80 + _index * 6;
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
            if (code.Type == 0xE0)
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

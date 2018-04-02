#include "CTRPluginFrameworkImpl/Menu/HexEditor.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFramework/Menu/Keyboard.hpp"

#include "3DS.h"
#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFrameworkImpl/Menu/Converter.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuActionReplay.hpp"

namespace CTRPluginFramework
{
    HexEditor *__g_hexEditor = nullptr;
    #define POS(x, y) (x | y << 16)

    // Start pos = (115, 82)
    //
    static const int    _cursorPositions[] =
    {
        POS(115, 82), POS(121, 82),     POS(136, 82), POS(142, 82),    POS(157, 82), POS(163, 82),  POS(178, 82), POS(184, 82),
        POS(206, 82), POS(212, 82),     POS(227, 82), POS(233, 82),    POS(248, 82), POS(254, 82),  POS(269, 82), POS(275, 82),

        POS(115, 92), POS(121, 92),     POS(136, 92), POS(142, 92),    POS(157, 92), POS(163, 92),  POS(178, 92), POS(184, 92),
        POS(206, 92), POS(212, 92),     POS(227, 92), POS(233, 92),    POS(248, 92), POS(254, 92),  POS(269, 92), POS(275, 92),

        POS(115, 102), POS(121, 102),     POS(136, 102), POS(142, 102),    POS(157, 102), POS(163, 102),  POS(178, 102), POS(184, 102),
        POS(206, 102), POS(212, 102),     POS(227, 102), POS(233, 102),    POS(248, 102), POS(254, 102),  POS(269, 102), POS(275, 102),

        POS(115, 112), POS(121, 112),     POS(136, 112), POS(142, 112),    POS(157, 112), POS(163, 112),  POS(178, 112), POS(184, 112),
        POS(206, 112), POS(212, 112),     POS(227, 112), POS(233, 112),    POS(248, 112), POS(254, 112),  POS(269, 112), POS(275, 112),

        POS(115, 122), POS(121, 122),     POS(136, 122), POS(142, 122),    POS(157, 122), POS(163, 122),  POS(178, 122), POS(184, 122),
        POS(206, 122), POS(212, 122),     POS(227, 122), POS(233, 122),    POS(248, 122), POS(254, 122),  POS(269, 122), POS(275, 122),

        POS(115, 132), POS(121, 132),     POS(136, 132), POS(142, 132),    POS(157, 132), POS(163, 132),  POS(178, 132), POS(184, 132),
        POS(206, 132), POS(212, 132),     POS(227, 132), POS(233, 132),    POS(248, 132), POS(254, 132),  POS(269, 132), POS(275, 132),

        POS(115, 142), POS(121, 142),     POS(136, 142), POS(142, 142),    POS(157, 142), POS(163, 142),  POS(178, 142), POS(184, 142),
        POS(206, 142), POS(212, 142),     POS(227, 142), POS(233, 142),    POS(248, 142), POS(254, 142),  POS(269, 142), POS(275, 142),

        POS(115, 152), POS(121, 152),     POS(136, 152), POS(142, 152),    POS(157, 152), POS(163, 152),  POS(178, 152), POS(184, 152),
        POS(206, 152), POS(212, 152),     POS(227, 152), POS(233, 152),    POS(248, 152), POS(254, 152),  POS(269, 152), POS(275, 152),

        POS(115, 162), POS(121, 162),     POS(136, 162), POS(142, 162),    POS(157, 162), POS(163, 162),  POS(178, 162), POS(184, 162),
        POS(206, 162), POS(212, 162),     POS(227, 162), POS(233, 162),    POS(248, 162), POS(254, 162),  POS(269, 162), POS(275, 162),

        POS(115, 172), POS(121, 172),     POS(136, 172), POS(142, 172),    POS(157, 172), POS(163, 172),  POS(178, 172), POS(184, 172),
        POS(206, 172), POS(212, 172),     POS(227, 172), POS(233, 172),    POS(248, 172), POS(254, 172),  POS(269, 172), POS(275, 172),
    };

    HexEditor::HexEditor(u32 target) :
        _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose),
        _submenu{ { "New cheat", "Jump to", "Jump relative", "Jump to value", "Converter", "Move backward", "Move forward", "Save this address", "Browse history", "Clear history" }}
    {
        // Init variables
        _invalid = true;
        _isModified = false;
        _startRegion = 0;
        _endRegion = 0;
        _cursor = 0;
        _indexHistory = -1;

        // Clean buffer
        memset(_memory, 0, 256);

        // Construct keyboard
        _keyboard.SetLayout(Layout::HEXADECIMAL);
        _keyboard._Hexadecimal();
        _keyboard._showCursor = false;

        __g_hexEditor = this;

        // Init memory etc
        Goto(target);
    }

    bool    HexEditor::operator()(EventList &eventList)
    {
        static bool keysAreDisabled = false;

        if (!keysAreDisabled)
        {
            // Disable clear key
            _keyboard._keys->at(15).Enable(false);
            // Disable enter key
            _keyboard._keys->at(16).Enable(false);
            keysAreDisabled = true;
        }

        // Update components (Only Close btn)
        _Update();

        // Process event
        bool isSubMenuOpen = _submenu.IsOpen();
        for (int i = 0; i < eventList.size(); i++)
        {
            _submenu.ProcessEvent(eventList[i]);
            if (!isSubMenuOpen)
                _ProcessEvent(eventList[i]);
        }

        int out;
        if (!_submenu.IsOpen() && _keyboard(out))
        {
            if (!_invalid)
            {
                _isModified = true;

                u8 value = (u8)out;
                if (_cursor % 2 == 0)
                {
                    value = (_memory[_cursor / 2] & 0b1111) | (value << 4);
                    _memory[_cursor / 2] = value;
                }
                else
                {
                    value = (_memory[_cursor / 2] & 0b11110000) | (value & 0b1111);
                    _memory[_cursor / 2] = value;
                }

                if (_cursor < 159)
                    _cursor++;
            }
        }
        else
        {
            int subchoice = _submenu();
            if (subchoice == 0) _CreateCheat();
            else if (subchoice == 1) _JumpTo(0);
            else if (subchoice == 2) _JumpTo(1);
            else if (subchoice == 3) _JumpTo(2);
            else if (subchoice == 4)
            {
                // Enable clear key
                _keyboard._keys->at(15).Enable(true);
                // Enable enter key
                _keyboard._keys->at(16).Enable(true);
                Converter *conv = Converter::Instance();

                if (conv) (*conv)();

                // Disable clear key
                _keyboard._keys->at(15).Enable(false);
                // Disable enter key
                _keyboard._keys->at(16).Enable(false);
            }
            else if (subchoice == 5) _MoveBackward();
            else if (subchoice == 6) _MoveForward();
            else if (subchoice == 7) _SaveThisAddress();
            else if (subchoice == 8) _BrowseHistory();
            else if (subchoice == 9) _ClearHistory();
        }

        // Render TopScreen
        _RenderTop();

        // Render Bottom Screen
        _RenderBottom();

        if (_closeBtn())
        {
            // Enable clear key
            _keyboard._keys->at(15).Enable(true);
            // Enable enter key
            _keyboard._keys->at(16).Enable(true);
            keysAreDisabled = false;
            return (true);
        }

        return (false);
    }

    void    HexEditor::_ProcessEvent(Event &event)
    {
        if (event.type == Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case Key::DPadLeft:
                {
                    if (_cursor > 15)
                    {
                        _cursor = std::max((int)(_cursor -1), (int)(_cursor & ~15));
                    }
                    else
                        _cursor = std::max((int)(_cursor -1), (int)(0));
                    break;
                }

                case Key::DPadRight:
                {
                    _cursor = std::min((int)(_cursor +1), (int)((_cursor & ~15) + 15));
                    break;
                }

                case Key::DPadUp:
                {
                    int oldcursor = _cursor;
                    _cursor = std::max((int)(_cursor - 16), (int)(_cursor & 15));

                    if (oldcursor == _cursor)
                    {
                        if ((u32)_memoryAddress > _startRegion)
                            Goto((u32)_memoryAddress - 8);
                    }
                    break;
                }

                case Key::DPadDown:
                {
                    int oldcursor = _cursor;
                    _cursor = std::min((int)(_cursor + 16), (int)((_cursor & 15) + 144));

                    if (oldcursor == _cursor)
                        Goto((u32)_memoryAddress + 8);
                    break;
                }

                case Key::A:
                {
                    if (_isModified)
                    {
                        _ApplyChanges();
                    }
                    break;
                }

                case Key::B:
                {
                    if (_isModified)
                    {
                        _DiscardChanges();
                    }
                    else
                        _closeBtn.SetState(true);
                    break;
                }
                case Key::L:
                {
                    _GotoPreviousRegion();
                    break;
                }

                case Key::R:
                {
                    _GotoNextRegion();
                    break;
                }
                default: break;
            }
        }
        static Clock timer;

        if (!_isModified && !_submenu.IsOpen() && event.type == Event::KeyDown && timer.HasTimePassed(Seconds(0.2f)))
        {
            timer.Restart();
            switch (event.key.code)
            {
                case Key::CPadUp:
                {
                    if ((u32)_memoryAddress > _startRegion)
                        Goto((u32)_memoryAddress - 8);
                    break;
                }

                case Key::CPadDown:
                {
                    Goto((u32)_memoryAddress + 8);
                    break;
                }
                case Key::CStickUp:
                {
                    if ((u32)_memoryAddress > _startRegion)
                        Goto((u32)_memoryAddress - 16);
                    break;
                }

                case Key::CStickDown:
                {
                    Goto((u32)_memoryAddress + 16);
                    break;
                }
                default: break;
            }
        }
    }

    #define OFF(d) ((i + d) * 8)

    void    HexEditor::_RenderTop(void)
    {
        Renderer::SetTarget(TOP);

        const Color     &black = Color::Black;
        const Color     &blank = Color::Blank;
        const Color     &skyblue = Color::SkyBlue;
        const Color     &deepskyblue = Color::DeepSkyBlue;
        const Color     &maintextcolor = Preferences::Settings.MainTextColor;
        const Color     &red = Color::Red;

        u32     address = (u32)_memoryAddress;
        u32     cursorAddress = address + ((_cursor / 16) * 8) + ((_cursor & 15) >> 1);

        char    buffer[0x100] = {0};

        int   posY = 61;

        Window::TopWindow.Draw("HexEditor");

        // Column headers

        // First column: 66px : 56 + 10
        // Second column: 178px : 168 + 10
        // last column: 66px : 56 + 10
        // Total:  310px
        // Margin between column: 10 px: 330px
        // Margin left / right: 5px : 340px

        // Selector address header
        Renderer::DrawRect(44, posY, 66, 20, skyblue);
        posY += 5;
        sprintf(buffer, "%08X", cursorAddress);
        Renderer::DrawString(buffer, 54, posY, black);
        posY += 6;
        // Address body
        Renderer::DrawRect(44, posY, 66, 100, deepskyblue);


        posY -= 21;
        // Values header
        Renderer::DrawRect(111, posY, 178, 20, deepskyblue);
        posY += 5;
        static const char * bytePos[] = { "00", "01", "02", "03", "04", "05", "06", "07" };
        // Values
        for (int i = 0, posXX = 116, posy = posY; i < 8; i++, posXX += 21)
        {
            posY = posy;
            Renderer::DrawString(bytePos[i], posXX, posY, black);
            if (i % 8 == 3)
                posXX += 7;
        }
        posY += 6;
        // Values body
        Renderer::DrawRect(111, posY, 178, 100, blank);

        posY -= 21;
        // characters header
        Renderer::DrawRect(290, posY, 66, 20, deepskyblue);
        // characters body
        posY += 21;
        Renderer::DrawRect(290, posY, 66, 100, skyblue);


        // Draw Cursor
        int cursorPos = _cursorPositions[_cursor];

        Renderer::DrawRect(cursorPos & 0xFFFF, cursorPos >> 16, 7, 10, skyblue);

        // Draw array

        if (_invalid)
        {
            for (int i = 0; i < 10; i++)
            {
                int posy = posY;

                // Address
                sprintf(buffer, "%08X", address);
                Renderer::DrawString(buffer, 54, posy, black);

                // Values
                for (int j = 0, posXX = 116; j < 8; j++, posXX += 21)
                {
                    posy = posY;
                    Renderer::DrawString("??", posXX, posy, black);
                    if (j % 8 == 3)
                        posXX += 7;
                }

                // Char
                Renderer::DrawString("........", 299, posY, black);

                address += 8;
            }
        }
        else
        {
            int     xPos = 116;
            int     yPos = posY - 10;

            // Address & data
            for (int i = 0; i < 10; i++)
            {
                int yy = posY;
                sprintf(buffer, "%08X", address);
                Renderer::DrawString(buffer, 54, posY, black);
                address += 8;

                _GetChar((u8 *)buffer, i * 8);
                Renderer::DrawString(buffer, 299, yy, black);
            }

            // Values
            for (int i = 0; i < 80; i++)
            {
                if (i % 8 == 0)
                {
                    xPos = 116;
                    yPos += 10;
                }

                posY = yPos;

                u8   original = _memoryAddress[i];
                u8   buf = _memory[i];
                bool diff = original != buf;

                if (!_isModified && diff)
                    _isModified = true;

                const Color &c = diff ? red : black;

                // Convert value
                sprintf(buffer, "%02X", buf);
                Renderer::DrawString(buffer, xPos, posY, c);

                xPos += 21;
                if (i % 8 == 3)
                    xPos += 7;
            }
        }

        if (_isModified)
        {
            posY += 5;
            Renderer::DrawString("Apply changes: ", 44, posY, maintextcolor);
            posY -= 14;
            Renderer::DrawSysString("\uE000", 149, posY, 330, maintextcolor);

            posY +=2;
            Renderer::DrawString("Discard changes: ", 44, posY, maintextcolor);
            posY -= 14;
            Renderer::DrawSysString("\uE001", 149, posY, 330, maintextcolor);
        }
        else
        {
            posY += 5;
            Renderer::DrawString("Options: ", 44, posY, maintextcolor);
            posY -= 14;
            Renderer::DrawSysString("\uE002", 99, posY, 330, maintextcolor);
        }
        _submenu.Draw();
    }

    void    HexEditor::_RenderBottom(void)
    {
        _keyboard._RenderBottom();

        _closeBtn.Draw();
    }

    void    HexEditor::_Update(void)
    {
        bool        isTouched = Touch::IsDown();
        IntVector   touchPos(Touch::GetPosition());

        _closeBtn.Update(isTouched, touchPos);
    }

    u32     HexEditor::_GetCursorAddress(void) const
    {
        u32     address = (u32)_memoryAddress;
        u32     cursorAddress = address + ((_cursor / 16) * 8) + ((_cursor & 15) >> 1);

        return (cursorAddress);
    }

    void    HexEditor::_CreateCheat(void)
    {
        if (!Preferences::Settings.AllowActionReplay)
            return;

        if (_invalid)
        {
            MessageBox("Error\n\nAddress invalid, abort")();
            return;
        }

        u32 address = _GetCursorAddress() & ~3;
        PluginMenuActionReplay::NewARCode(0, address, *(u32 *)address);
    }

    void    HexEditor::_MoveBackward(void)
    {
        // If there's nothing in the history
        if (!_history.size())
            return;

        --_indexHistory;
        if (_indexHistory <= -1)
            _indexHistory = 0;

        // Jump to address
        Goto(_history[_indexHistory], true);
    }

    void    HexEditor::_MoveForward(void)
    {
        // If there's nothing in the history
        if (!_history.size())
            return;

        ++_indexHistory;
        if (_indexHistory >= _history.size() - 1)
            _indexHistory = _history.size() - 1;

        // Jump to address
        Goto(_history[_indexHistory], true);
    }

    void    HexEditor::_SaveThisAddress(void)
    {
        u32 address = _GetCursorAddress();
        if (_history.size() && _history.back() == address)
            return;
        _history.push_back(address);
        _indexHistory = _history.size() - 1;
    }

    void    HexEditor::_BrowseHistory(void)
    {
        if (_history.empty())
        {
            MessageBox("Error\n\nHistory is empty")();
            return;
        }

        Keyboard    keyboard;
        std::vector<std::string> addresses;

        for (u32 v : _history)
            addresses.push_back(Utils::ToHex(v));

        keyboard.DisplayTopScreen = false;
        keyboard.Populate(addresses);

        int index = keyboard.Open();

        if (index != -1)
        {
            _indexHistory = index;
            Goto(_history[index], true);
        }
    }

    void    HexEditor::_ClearHistory(void)
    {
        _history.clear();
        _indexHistory = -1;
    }

    void    HexEditor::Goto(u32 address, bool updateCursor)
    {
        MemInfo mInfo;
        PageInfo pInfo;

        u32 addrBak = address;

        address &= ~3;
        if (address % 8)
            address -= 4;

        if (address >= _startRegion && address < _endRegion)
        {
            if (address + 80 >= _endRegion)
                address = _endRegion - 80;

            if (updateCursor)
                _cursor = (addrBak - address) * 2;

            _memoryAddress = reinterpret_cast<u8 *>(address);

            _invalid = !Process::CopyMemory(_memory, reinterpret_cast<void *>(address), 80);

            return;
        }

        if (R_SUCCEEDED(svcQueryProcessMemory(&mInfo, &pInfo, Process::GetHandle(), address)))
        {
            if (mInfo.state == 0 || mInfo.state == 2 || mInfo.state == 11)
                goto invalid;

            if ((mInfo.perm & (MEMPERM_READ | MEMPERM_WRITE) != MEMPERM_READ | MEMPERM_WRITE))
            {
                if (!Process::ProtectMemory(mInfo.base_addr, mInfo.size, mInfo.perm | (MEMPERM_READ | MEMPERM_WRITE)))
                    goto invalid;
            }
        }
        else
            goto invalid;

    copy:
        if (!Process::CopyMemory(_memory, reinterpret_cast<void *>(address), 80))
            goto invalid;

        _memoryAddress = reinterpret_cast<u8 *>(address);
        _invalid = false;
        _startRegion = mInfo.base_addr;
        _endRegion = _startRegion + mInfo.size;
        _cursor = (addrBak - address) * 2;

        return;

    invalid:
        _memoryAddress = reinterpret_cast<u8 *>(address);
        _invalid = true;
        _startRegion = mInfo.base_addr;
        _endRegion = _startRegion + mInfo.size;
        _cursor = (addrBak - address) * 2;
    }

    void    HexEditor::_ApplyChanges(void)
    {
        svcFlushProcessDataCache(Process::GetHandle(), (void *)_memoryAddress, 80);
        svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_memoryAddress, 80);
        std::memcpy(_memoryAddress, _memory, 80);
        _isModified = false;
    }

    void    HexEditor::_DiscardChanges(void)
    {
        _isModified = false;
        svcFlushProcessDataCache(Process::GetHandle(), (void *)_memoryAddress, 80);
        std::memcpy(_memory, _memoryAddress, 80);
    }

    u32     HexEditor::_PromptForAddress(int mode)
    {
        static const char *msg[2] = {"Enter the address to jump to:", "Enter the offset to jump to:"};

        Keyboard    keyboard;

        const Color     &bgMain = Preferences::Settings.BackgroundMainColor;
        const Color     &bgSecondary = Preferences::Settings.BackgroundSecondaryColor;
        const Color     &skyblue = Color::SkyBlue;
        static IntRect  background(93, 95, 213, 50);

        Renderer::SetTarget(TOP);

        // Draw "window" background
        Renderer::DrawRect2(background, bgMain, bgSecondary);

        int posY = 115;
        Renderer::DrawString(msg[mode], 98, posY, skyblue);

        // We write to second framebuffer too because of keyboard below
        Renderer::EndFrame();

        Renderer::SetTarget(TOP);
        Renderer::DrawRect2(background, bgMain, bgSecondary);
        posY = 115;
        Renderer::DrawString(msg[mode], 98, posY, skyblue);

        keyboard.DisplayTopScreen = false;

        u32 address = mode == 0 ? (u32)_memoryAddress : 0;

        // Enable clear key
        _keyboard._keys->at(15).Enable(true);
        // Enable enter key
        _keyboard._keys->at(16).Enable(true);
        // Enable cursor
        _keyboard._showCursor = true;

        // If user aborted, return 0
        if (keyboard.Open(address, address) == -1)
            address = 0;
        else if (mode == 1)
            address += (u32)_memoryAddress;

        // Disable clear key
        _keyboard._keys->at(15).Enable(false);
        // Disable enter key
        _keyboard._keys->at(16).Enable(false);
        // Disable cursor
        _keyboard._showCursor = false;

        return address;
    }
    void    HexEditor::_JumpTo(int mode)
    {
        u32 address;

        if (mode == 2)
        {
            if (!Process::Read32(_GetCursorAddress(), address))
                return;

            if (!(MessageBox("Confirm",
                Utils::Format("Do you want to jump to: 0x%08X ?", address), DialogType::DialogYesNo))())
                return;
        }
        else if (mode < 2)
        {
            address = _PromptForAddress(mode);
            if (!address)
                return;
        }

        Goto(address, true);
        _history.push_back(address);
        _indexHistory = _history.size() - 1;
    }

    void    HexEditor::_GotoPreviousRegion(void)
    {
        if (_isModified || _submenu.IsOpen())
            return;

        if (_startRegion > 0)
        {
            u32 address = _startRegion - 8;

            Goto(address);
            Goto(_startRegion);
            _cursor = 0;
        }
    }

    void    HexEditor::_GotoNextRegion(void)
    {
        if (_isModified || _submenu.IsOpen())
            return;

        u32 address = _endRegion + 8;

        if (address < 0x40000000)
        {
            Goto(address);
            Goto(_startRegion);
            _cursor = 0;
        }
    }

    void    HexEditor::_GetChar(u8 *buffer, int offset)
    {
        for (int i = 0; i < 8; i++)
        {
            u8 c = _memory[i + offset];

            if (c >= 32 && c <= 126)
                buffer[i] = c;
            else
                buffer[i] = '.';
        }
    }
}

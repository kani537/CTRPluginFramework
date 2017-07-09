#include "CTRPluginFrameworkImpl/Menu/HexEditor.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFramework/Menu/Keyboard.hpp"

#include "3DS.h"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuFreeCheats.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"

namespace CTRPluginFramework
{   
    #define POS(x, y) (x | y << 16)
    
    // Start pos = (115, 82)
    //
    static const int    _cursorPositions[] =
    {
        POS(115, 82), POS(122, 82),     POS(136, 82), POS(143, 82),    POS(157, 82), POS(164, 82),  POS(178, 82), POS(185, 82),
        POS(206, 82), POS(213, 82),     POS(227, 82), POS(234, 82),    POS(248, 82), POS(255, 82),  POS(269, 82), POS(276, 82),

        POS(115, 92), POS(122, 92),     POS(136, 92), POS(143, 92),    POS(157, 92), POS(164, 92),  POS(178, 92), POS(185, 92),
        POS(206, 92), POS(213, 92),     POS(227, 92), POS(234, 92),    POS(248, 92), POS(255, 92),  POS(269, 92), POS(276, 92),

        POS(115, 102), POS(122, 102),     POS(136, 102), POS(143, 102),    POS(157, 102), POS(164, 102),  POS(178, 102), POS(185, 102),
        POS(206, 102), POS(213, 102),     POS(227, 102), POS(234, 102),    POS(248, 102), POS(255, 102),  POS(269, 102), POS(276, 102),

        POS(115, 112), POS(122, 112),     POS(136, 112), POS(143, 112),    POS(157, 112), POS(164, 112),  POS(178, 112), POS(185, 112),
        POS(206, 112), POS(213, 112),     POS(227, 112), POS(234, 112),    POS(248, 112), POS(255, 112),  POS(269, 112), POS(276, 112),

        POS(115, 122), POS(122, 122),     POS(136, 122), POS(143, 122),    POS(157, 122), POS(164, 122),  POS(178, 122), POS(185, 122),
        POS(206, 122), POS(213, 122),     POS(227, 122), POS(234, 122),    POS(248, 122), POS(255, 122),  POS(269, 122), POS(276, 122),

        POS(115, 132), POS(122, 132),     POS(136, 132), POS(143, 132),    POS(157, 132), POS(164, 132),  POS(178, 132), POS(185, 132),
        POS(206, 132), POS(213, 132),     POS(227, 132), POS(234, 132),    POS(248, 132), POS(255, 132),  POS(269, 132), POS(276, 132),

        POS(115, 142), POS(122, 142),     POS(136, 142), POS(143, 142),    POS(157, 142), POS(164, 142),  POS(178, 142), POS(185, 142),
        POS(206, 142), POS(213, 142),     POS(227, 142), POS(234, 142),    POS(248, 142), POS(255, 142),  POS(269, 142), POS(276, 142),

        POS(115, 152), POS(122, 152),     POS(136, 152), POS(143, 152),    POS(157, 152), POS(164, 152),  POS(178, 152), POS(185, 152),
        POS(206, 152), POS(213, 152),     POS(227, 152), POS(234, 152),    POS(248, 152), POS(255, 152),  POS(269, 152), POS(276, 152),

        POS(115, 162), POS(122, 162),     POS(136, 162), POS(143, 162),    POS(157, 162), POS(164, 162),  POS(178, 162), POS(185, 162),
        POS(206, 162), POS(213, 162),     POS(227, 162), POS(234, 162),    POS(248, 162), POS(255, 162),  POS(269, 162), POS(276, 162),

        POS(115, 172), POS(122, 172),     POS(136, 172), POS(143, 172),    POS(157, 172), POS(164, 172),  POS(178, 172), POS(185, 172),
        POS(206, 172), POS(213, 172),     POS(227, 172), POS(234, 172),    POS(248, 172), POS(255, 172),  POS(269, 172), POS(276, 172),
    };

    HexEditor::HexEditor(u32 target) :
    _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose)
    {
        // Init variables
        _invalid = true;
        _isModified = false;
        _subMenuOpen = false;
        _action = false;
        _startRegion = 0;
        _endRegion = 0;
        _cursor = 0;
        _subCursor = 0;
        _indexHistory = -1;

        // Clean buffer
        memset(_memory, 0, 256);

        // Construct keyboard
        _keyboard.SetLayout(Layout::HEXADECIMAL);
        _keyboard._Hexadecimal();
        // Disable backspace key
        _keyboard._keys.at(15).Enable(false);
        // Disable enter key
        _keyboard._keys.at(16).Enable(false);

        // Create options
        _options.push_back("New FreeCheat");
        _options.push_back("Jump to");
        _options.push_back("Move backward");
        _options.push_back("Move forward");
        _options.push_back("Save this address");
        _options.push_back("Browse history");
        _options.push_back("Clear history");

        // Init memory etc
        Goto(target);
    }

    bool    HexEditor::operator()(EventList &eventList)
    {
        // Process event
        for (int i = 0; i < eventList.size(); i++)
            _ProcessEvent(eventList[i]);

        // Update components
        _Update();

        int out;
        if (!_subMenuOpen && _keyboard(out))
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

        // Render TopScreen
        _RenderTop();

        // Render Bottom Screen
        _RenderBottom();

        if (_closeBtn())
            return (true);

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
                    if (_subMenuOpen)
                        break;
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
                    if (_subMenuOpen)
                        break;
                    _cursor = std::min((int)(_cursor +1), (int)((_cursor & ~15) + 15));
                    break;
                }

                case Key::DPadUp:
                {
                    if (_subMenuOpen)
                        _subCursor = std::max((int)(_subCursor - 1), 0);
                    else
                        _cursor = std::max((int)(_cursor - 16), (int)(_cursor & 15));
                    break;
                }

                case Key::DPadDown:
                {
                    if (_subMenuOpen)
                        _subCursor = std::min((int)(_subCursor + 1), (int)(_options.size() - 1));
                    else
                        _cursor = std::min((int)(_cursor + 16), (int)((_cursor & 15) + 144));
                    break;
                }

                case Key::A:
                {
                    if (_subMenuOpen)
                    {
                        if (_subCursor == 0) _CreateFreeCheat();
                        else if (_subCursor == 1) _JumpTo();
                        else if (_subCursor == 2) _MoveBackward();
                        else if (_subCursor == 3) _MoveForward();
                        else if (_subCursor == 4) _SaveThisAddress();
                        else if (_subCursor == 5) _BrowseHistory();
                        else if (_subCursor == 6) _ClearHistory();
                    }
                    else if (_isModified)
                    {
                        _ApplyChanges();
                    }
                    break;
                }

                case Key::B:
                {
                    if (_subMenuOpen)
                    {
                        _subMenuOpen = false;
                    }
                    else if (_isModified)
                    {
                        _DiscardChanges();
                    }
                    break;
                }

                case Key::X:
                {
                    if (!_isModified && !_subMenuOpen)
                    {
                        _subMenuOpen = true;
                        break;
                    }
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

        if (!_isModified && !_subMenuOpen && event.type == Event::KeyDown && timer.HasTimePassed(Seconds(0.2f)))
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
                default: break;
            }  
        }
    }

    #define OFF(d) ((i + d) * 8)

    void    HexEditor::_DrawSubMenu(void)
    {
        Color    &black = Color::Black;
        Color    &blank = Color::Blank;
        Color    &dimGrey = Color::BlackGrey;
        Color    &darkgrey = Color::DarkGrey;
        Color    &gainsboro = Color::Gainsboro;
        Color    &skyblue = Color::SkyBlue;
        static IntRect  background(240, 20, 130, 200);

        // DrawBackground
        Renderer::DrawRect2(background, black, dimGrey);

        int posY = 25;

        // Draw title's menu
        int xx = Renderer::DrawSysString("Options", 245, posY, 340, blank);
        Renderer::DrawLine(245, posY, xx - 225, skyblue);

        posY = 46;

        IntRect selRect = IntRect(241, 45 + _subCursor * 12, 125, 12);

        for (int i = 0; i < _options.size(); i++)
        {
            std::string &str = _options[i];

            if (i == _subCursor)
            {
                if (_action || !_buttonFade.HasTimePassed(Seconds(0.2f)))
                {
                    if (_action)
                    {
                        _action = false;
                        _buttonFade.Restart();
                    }

                    // Draw action rectangle
                    Renderer::DrawRect(selRect, gainsboro);
                    // Draw selector
                    Renderer::DrawRect(selRect, darkgrey, false);
                    // Draw text
                    Renderer::DrawString((char *)str.c_str(), 245, posY, black);
                }
                else
                {
                    // Draw selector
                    Renderer::DrawRect(selRect, darkgrey, false);

                    // Draw text
                    Renderer::DrawString((char *)str.c_str(), 245, posY, blank);
                }
            }
            else
            {
                Renderer::DrawString((char *)str.c_str(), 245, posY, blank);
            }
            posY += 2;
        }
    }
    void    HexEditor::_RenderTop(void)
    {
        Renderer::SetTarget(TOP);

        Color    &black = Color::Black;
        Color    &blank = Color::Blank;
        Color    &skyblue = Color::SkyBlue;
        Color    &deepskyblue = Color::DeepSkyBlue;
        Color    &dodgerblue = Color::DodgerBlue;
        Color    &red = Color::Red;

        u32     address = (u32)_memoryAddress;
        u32     cursorAddress = address + ((_cursor / 16) * 8) + ((_cursor & 15) >> 1);

        char    buffer[0x100] = {0};

        int   posY = 25;
        int   posX = 40;

        Window::TopWindow.Draw();

        // Title
        int xx = Renderer::DrawSysString("HexEditor", posX, posY, 300, blank);
        Renderer::DrawLine(posX, posY, xx, dodgerblue);

        
        posY += 20;

        // Column headers

        // First column: 66px : 56 + 10
        // Second column: 178px : 168 + 10
        // last column: 66px : 56 + 10
        // Total:  310px
        // MArgin between column: 10 px: 330px
        // Margin left / right: 5px : 340px

        // Selector address header 
        Renderer::DrawRect(44, posY, 66, 20, skyblue);
        posY += 5;
        sprintf(buffer, "%08X", cursorAddress);
        Renderer::DrawString(buffer, 50, posY, black);
        posY += 6;
        // Address body
        Renderer::DrawRect(44, posY, 66, 100, deepskyblue);


        posY -= 21;
        // Values header
        Renderer::DrawRect(111, posY, 178, 20, deepskyblue);
        posY += 5;
        Renderer::DrawString((char *)"00 01 02 03  04 05 06 07", 116, posY, black);
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
                int posyy = posY;

                // Address
                sprintf(buffer, "%08X", address);
                Renderer::DrawString(buffer, 50, posY, black);

                // Values
                Renderer::DrawString((char *)"?? ?? ?? ??  ?? ?? ?? ??", 116, posy, black);

                // Char
                Renderer::DrawString((char *)"........", 295, posyy, black);

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
                Renderer::DrawString(buffer, 50, posY, black);
                address += 8;

                _GetChar((u8 *)buffer, i * 8);
                Renderer::DrawString(buffer, 295, yy, black);
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

                Color &c = diff ? red : black;

                // Convert value
                sprintf(buffer, "%02X ", buf);
                Renderer::DrawString(buffer, xPos, posY, c);

                xPos += 21;
                if (i % 8 == 3)
                    xPos += 7;                
            } 
        }
        
        if (_isModified)
        {
            posY += 5;
            Renderer::DrawString((char *)"Apply changes: ", 44, posY, blank);
            posY -= 14;
            Renderer::DrawSysString("\uE000", 149, posY, 330, blank);

            posY +=2;
            Renderer::DrawString((char *)"Discard changes: ", 44, posY, blank);
            posY -= 14;
            Renderer::DrawSysString("\uE001", 163, posY, 330, blank);
        }
        else
        {
            posY += 5;
            Renderer::DrawString((char *)"Options: ", 44, posY, blank);
            posY -= 14;
            Renderer::DrawSysString("\uE002", 99, posY, 330, blank);  
        }

        if (_subMenuOpen)
        {
            _DrawSubMenu();
            return;
        }
        
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

    void    HexEditor::_CreateFreeCheat(void)
    {
        _action = true;

        if (_invalid)
        {
            MessageBox("Error\n\nAddress invalid, abort")();
            return;
        }

        u32 address = _GetCursorAddress() & ~3;
        FreeCheats::GetInstance()->Create(address, *(u32 *)address);
    }

    void    HexEditor::_MoveBackward(void)
    {
        _action = true;

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
        _action = true;

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
        _action = true;

        u32 address = _GetCursorAddress();
        if (_history.back() == address)
            return;
        _history.push_back(address);
        _indexHistory = _history.size() - 1;
    }

    std::string ToHex(u32 x);

    void    HexEditor::_BrowseHistory(void)
    {
        _action = true;
        
        if (_history.empty())
        {
            MessageBox("Error\n\nHistory is empty")();
            return;
        }

        Keyboard    keyboard;
        std::vector<std::string> addresses;

        for (u32 v : _history)
            addresses.push_back(ToHex(v));

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
        _action = true;

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

        if (address >= _startRegion && address <= _endRegion)
        {
            if (address + 80 > _endRegion)
                address = _endRegion - 80;

            if (updateCursor)
                _cursor = (addrBak - address) * 2;

            _memoryAddress = (u8 *)address;            

            _invalid = !Process::CopyMemory(_memory, (void *)address, 80);

            return;
        }

        if (R_SUCCEEDED(svcQueryProcessMemory(&mInfo, &pInfo, Process::GetHandle(), address)))
        {
            if (mInfo.state == 0)
            {
                goto invalid;
            }

            if ((mInfo.perm & (MEMPERM_READ | MEMPERM_WRITE) != MEMPERM_READ | MEMPERM_WRITE))
            {
                if (!Process::ProtectMemory(mInfo.base_addr, mInfo.size, mInfo.perm | (MEMPERM_READ | MEMPERM_WRITE)))
                {
                    goto invalid;
                }
            }
        }
        else
            goto invalid;
        
    copy:
        //svcFlushProcessDataCache(Process::GetHandle(), (void *)address, 80);

        if (!Process::CopyMemory(_memory, (void *)address, 80))
            goto invalid;

        _memoryAddress = (u8 *)address;
        _invalid = false;
        _startRegion = mInfo.base_addr;
        _endRegion = _startRegion + mInfo.size;
        _cursor = (addrBak - address) * 2;

        return;
    invalid:
        _memoryAddress = (u8 *)address;
        _invalid = true;
        _startRegion = mInfo.base_addr;
        _endRegion = _startRegion + mInfo.size;
        _cursor = (addrBak - address) * 2;
        return;

    }

    void    HexEditor::_ApplyChanges(void)
    {
        std::memcpy(_memoryAddress, _memory, 80);
        svcFlushProcessDataCache(Process::GetHandle(), (void *)_memoryAddress, 80);
        svcInvalidateProcessDataCache(Process::GetHandle(), (void *)_memoryAddress, 80);
        _isModified = false;
    }

    void    HexEditor::_DiscardChanges(void)
    {
        _isModified = false;
        svcFlushProcessDataCache(Process::GetHandle(), (void *)_memoryAddress, 80);
        std::memcpy(_memory, _memoryAddress, 80);
    }

    void    HexEditor::_JumpTo(void)
    {
        _action = true;
        Keyboard    keyboard;

        Color    &black = Color::Black;
        Color    &dimGrey = Color::BlackGrey;
        Color    &skyblue = Color::SkyBlue;
        static IntRect  background(93, 95, 213, 50);
        

        Renderer::SetTarget(TOP);

        // Draw "window" background
        Renderer::DrawRect2(background, black, dimGrey);

        int posY = 115;

        Renderer::DrawString((char *)"Enter the address to jump to:", 98, posY, skyblue);

        Renderer::EndFrame();

        Renderer::SetTarget(TOP);

        // Draw "window" background
        Renderer::DrawRect2(background, black, dimGrey);

        posY = 115;

        Renderer::DrawString((char *)"Enter the address to jump to:", 98, posY, skyblue);


        keyboard.DisplayTopScreen = false;

        u32 address = (u32)_memoryAddress;
        if (keyboard.Open(address, address) != -1)
        {
            Goto(address, true);
            _history.push_back(address);
            _indexHistory = _history.size() - 1;
        }
    }

    void    HexEditor::_GotoPreviousRegion(void)
    {
        if (_isModified || _subMenuOpen)
            return;

        if (_startRegion > 0)
        {
            u32 address = _startRegion - 8;

            if (address >= 0x10000000 && address < 0x14000000)
                address = 0x10000000 - 8;

            Goto(address);
            _cursor = 0;
            Goto(_startRegion);
            _cursor = 0;
        }
    }

    void    HexEditor::_GotoNextRegion()
    {
        if (_isModified || _subMenuOpen)
            return;

        u32 address = _endRegion + 8;

        if (address < 0x50000000)
        {
            if (address >= 0x10000000 && address < 0x14000000)
                address = 0x14000000;

            Goto(address);
            _cursor = 0;
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

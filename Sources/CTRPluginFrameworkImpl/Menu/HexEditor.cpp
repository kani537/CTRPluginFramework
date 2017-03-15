#include "CTRPluginFrameworkImpl/Menu/HexEditor.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    HexEditor::HexEditor(u32 target) :
    _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose)
    {
        // Init variables
        _invalid = false;
        _isModified = false;
        _cursor = 0;

        // Clean buffer
        memset(_memory, 0, 256);

        // Construct keyboard
        _keyboard.SetLayout(Layout::HEXADECIMAL);
        _keyboard._Hexadecimal();
        // Disable backspace key
        _keyboard._keys.at(15).Enable(false);
        // Disable enter key
        _keyboard._keys.at(16).Enable(false);

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
        if (_keyboard(out))
        {
            _isModified = true;
            // Handle out

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

    }

    #define OFF(d) ((i + d) * 8)
    void    HexEditor::_RenderTop(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static Color    silver(160, 160, 160);
        static IntRect  background(30, 20, 340, 200);

        int   posY = 25;
        int   posX = 40;

        Renderer::SetTarget(TOP);

        // Draw background
        if (Preferences::topBackgroundImage->IsLoaded())
            Preferences::topBackgroundImage->Draw(background.leftTop);
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            Renderer::DrawRect(32, 22, 336, 196, blank, false);
        }

        // Title
        int xx = Renderer::DrawSysString("HexEditor", posX, posY, 300, blank);
        Renderer::DrawLine(posX, posY, xx, blank);

        posY += 10;

        // Draw Cursor
        // TODO

        // Draw array
        u32     address = (u32)_memoryAddress;
        char    buffer[0x100] = {0};

        for (int i = 0; i < 10; i++)
        {
            // Convert value
            sprintf(buffer, "%08X : %02X %02X %02X %02X  %02X %02X %02X %02X", address, 
                    _memory[OFF(0)], _memory[OFF(1)], _memory[OFF(2)], _memory[OFF(3)],
                    _memory[OFF(4)], _memory[OFF(5)], _memory[OFF(6)], _memory[OFF(7)]);

            std::string str = buffer;
            str += _GetChar(i);

            // Draw it
            Renderer::DrawString((char *)str.c_str(), posX, posY, blank);

            address += 8;
        }

        if (_isModified)
        {
            Renderer::DrawSysString("\uE000: Apply changes  \uE001: Discard changes", posX, posY, 330, blank);
        }
    }

    void    HexEditor::_RenderBottom(void)
    {
        _keyboard._RenderBottom();

        _closeBtn.Draw();
        /*static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static IntRect  background(20, 20, 280, 200);
        static Clock    creditClock;
        static bool     framework = true;

        Renderer::SetTarget(BOTTOM);*/

        /*// Background
        if (Preferences::bottomBackgroundImage->IsLoaded())
            Preferences::bottomBackgroundImage->Draw(background.leftTop);
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            Renderer::DrawRect(22, 22, 276, 196, blank, false); 
        }*/
    }

    void    HexEditor::_Update(void)
    {
        bool        isTouched = Touch::IsDown();
        IntVector   touchPos(Touch::GetPosition());

        _closeBtn.Update(isTouched, touchPos);
    }

    void    HexEditor::Goto(u32 address)
    {
       /* bool retry = false;

        address &= ~3;
        if (address % 8)
            address -= 4;
        if (!Process::CopyMemory(_modified, (void *)address, 80));
        {
            MemInfo mInfo;
            PageInfo pInfo;

            svcQueryProcessMemoryInfo(Process::GetHandle(), &mInfo, &pInfo, address);

            if (mInfo.state != 0)
            {
                address = (mInfo.base_addr + mInfo.size) - 80;
            }
            _invalid = true;
            return;
        }*/

    }

    void    HexEditor::_ApplyChanges(void)
    {

    }

    void    HexEditor::_DiscardChanges(void)
    {

    }

    std::string     HexEditor::_GetChar(int offset)
    {
        std::string ret = "";

        for (int i = 0; i < 8; i++)
        {
            u8 c = _memory[i + offset];

            if (c >= 32 && c <= 126)
                ret += c;
            else
                ret += '.';
        }

        return (ret);
    }
}
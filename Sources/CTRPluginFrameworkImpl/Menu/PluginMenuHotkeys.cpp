#include "CTRPluginFrameworkImpl/Menu/PluginMenuHotkeys.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Window.hpp"
#include "CTRPluginFramework/System/Controller.hpp"
#include "Unicode.h"

namespace CTRPluginFramework
{
    static const Key ktable[14] = 
    {
        ZL, L, DPadUp, DPadLeft, DPadRight, DPadDown, Start,
        ZR, R, X, Y, A, B, Select
    };

    static const char *stable[14] =
    {
        FONT_ZL, FONT_L, FONT_DU, FONT_DL, FONT_DR, FONT_DD, "Start",
        FONT_ZR, FONT_R, FONT_X, FONT_Y, FONT_A, FONT_B, "Select"
    };

    static int    GetIndex(int code)
    {
        for (int i = 0; i < 14; ++i)
            if (ktable[i] == code)
                return (i);

        return (0); ///< shouldn"t happen (unsafe code, bad)
    }

    HotkeysModifier::HotkeysModifier(u32 &keys, const std::string &message) :
    _keys(keys), _message(message)
    {
        for (int i = 0, posY = 32; i < 7; ++i, posY += 25)
            _checkboxs.push_back(CheckBox(30, posY));
        for (int i = 0, posY = 32; i < 7; ++i, posY += 25)
            _checkboxs.push_back(CheckBox(200, posY));

        for (int i = 0; i < 16; ++i)
        {
            if (keys & (1u << i))
            {
                _checkboxs[GetIndex(1u << i)].SetState(true);
            }
        }
    }

    HotkeysModifier::~HotkeysModifier()
    {
    }

    void    HotkeysModifier::operator()(void)
    {
        while (!Window::BottomWindow.MustClose())
        {
            Controller::Update();
            _DrawTop();
            _DrawBottom();
            Renderer::EndFrame();
            _Update();
        }
        
        u32 keys = 0;

        for (int i = 0; i < 14; i++)
        {
            if (_checkboxs[i].IsChecked())
                keys |= ktable[i];
        }

        _keys = keys;
    }

    void    HotkeysModifier::_DrawTop(void) const
    {
        Renderer::SetTarget(TOP);
        Window::TopWindow.Draw();

        int posY = 25;
        int xx = Renderer::DrawSysString("Hotkey Modifier", 40, posY, 330, Color::Blank);
        Renderer::DrawLine(40, posY, xx, Color::Blank);

        posY += 20;
        Renderer::DrawSysStringReturn((const u8*)_message.c_str(), 40, posY, 300, Color::Blank);
    }

    void    HotkeysModifier::_DrawBottom(void)
    {
        Renderer::SetTarget(BOTTOM);
        Window::BottomWindow.Draw();

        // Draw CheckBoxes
        for (CheckBox &cb : _checkboxs)
            cb.Draw();

        // Draw labels
        for (int i = 0, posY = 32; i < 7; ++i, posY += 9)
            Renderer::DrawSysString(stable[i], 50, posY, 290, Color::Blank);
        for (int i = 7, posY = 32; i < 14; ++i, posY += 9)
            Renderer::DrawSysString(stable[i], 220, posY, 290, Color::Blank);
    }

    void    HotkeysModifier::_Update(void)
    {
        bool        isTouched = Touch::IsDown();
        IntVector   touchPos(Touch::GetPosition());

        for (CheckBox &cb : _checkboxs)
            cb.Update(isTouched, touchPos);

        Window::BottomWindow.Update(isTouched, touchPos);
    }
}

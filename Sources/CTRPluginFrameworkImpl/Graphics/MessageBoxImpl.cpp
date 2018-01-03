#include "CTRPluginFrameworkImpl/Graphics/MessageBoxImpl.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFrameworkImpl/System/EventManager.hpp"
#include "CTRPluginFramework/Menu/PluginMenu.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "CTRPluginFramework/System/FwkSettings.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    MessageBoxImpl::MessageBoxImpl(const std::string &title, const std::string &message, DialogType dType) :
        _title(title), _message(message), _dialogType(dType), _exit(false), _cursor(0)
    {
        /*
        * line = 16px
        * buttons = 20px
        * Top Border = 10px
        * Text to buttons border = 13px
        * Title = 16 + 8 = 24px
        * Buttons to bottom broder = 5px
        * Total after text border = 13 + 20 + 5 = 38px
        * Total border = 10 + 38 = 48px
        * Min height for 1 line: 48px + 16px = 64px
        * Min height for title + 1 line: 48px + 16px + 24px = 88px
        */

        int     minWidth = dType == DialogType::DialogOk ? 100 : 200;
        int     lineCount;
        float   maxLineWidth;
        float   maxTitleWidth;

        // Check if a color is defined in the string of the title
        if (title[0] == 0x1B)
        {
            _titleColor.r = title[1];
            _titleColor.g = title[2];
            _titleColor.b = title[3];
        }
        else
            _titleColor = Preferences::Settings.WindowTitleColor;

        Renderer::GetTextInfos(title.c_str(), lineCount, maxTitleWidth, 320.f);
        Renderer::GetTextInfos(message.c_str(), lineCount, maxLineWidth, 320.f);
        maxLineWidth = std::max(maxLineWidth, maxTitleWidth) + 10.f; /// Size + potential scrollbar

        if (lineCount > 10)
            lineCount = 10;

        int height = 16 * lineCount + 72;
        int posY = (240 - height) / 2;
        int width = std::max((int)(maxLineWidth + 11.f), minWidth);
        int posX = (400 - width) / 2;

        _box = IntRect(posX, posY, width, height - 25);

        _textbox._box = _box;
        _textbox._border = IntRect(_box.leftTop.x + 2, _box.leftTop.y + 2, _box.size.x - 4, _box.size.y - 4);
        _box.size.y = height;
        _textbox.Update(_title, _message);
        _textbox.Open();
        _textbox.titleColor = _titleColor;
        _textbox._fastscroll = _textbox._drawBox = false;
    }
    MessageBoxImpl::MessageBoxImpl(const std::string &message, const DialogType dType) :
        _message(message), _dialogType(dType), _exit(false), _cursor(0)
    {
        int     minWidth = dType == DialogType::DialogOk ? 100 : 200;
        int     lineCount;
        float   maxLineWidth;

        Renderer::GetTextInfos(message.c_str(), lineCount, maxLineWidth, 290.f);

        maxLineWidth += 10.f; ///< Add potential scrollbar size
        if (lineCount > 10)
            lineCount = 10;

        int height = 16 * lineCount + 48;
        int posY = (240 - height) / 2;
        int width = std::max((int)(maxLineWidth + 11), minWidth);
        int posX = (400 - width) / 2;

        _box = IntRect(posX, posY, width, height - 25);
        _textbox._box = _box;
        _textbox._border = IntRect(_box.leftTop.x + 2, _box.leftTop.y + 2, _box.size.x - 4, _box.size.y - 4);
        _box.size.y = height;
        _textbox.Update(_title, _message);
        _textbox.Open();
        _textbox.titleColor = _titleColor;
        _textbox._fastscroll = _textbox._drawBox = false;
    }

    bool    MessageBoxImpl::operator()(void)
    {
        bool mustReleaseGame = false;

        // Check if game is paused
        if (!ProcessImpl::IsPaused())
        {
            mustReleaseGame = true;
            ProcessImpl::Pause(false);
        }

        Event               event;
        EventManager        manager;

        // While user didn't close the MessageBox
        while (!_exit)
        {
            // Process Events
            while (manager.PollEvent(event))
            {
                _ProcessEvent(event);
                _textbox.ProcessEvent(event);
            }

            if (_exit)
                break;

            // Draw box
            _Draw();
            Renderer::SetTarget(BOTTOM);
            Renderer::EndFrame();
        }

        // Wait until keys are released
        while (true)
        {
            Controller::Update();
            if (!Controller::IsKeyDown(Key::A) && !Controller::IsKeyDown(Key::B))
                break;
        }

        // Release game if we paused it in this function
        PluginMenu *menu = PluginMenu::GetRunningInstance();
        if (menu && !menu->IsOpen())
            ScreenImpl::Clean();
        if (mustReleaseGame)
            ProcessImpl::Play(false);

        return (_cursor ? false : true);
    }

    void    MessageBoxImpl::_ProcessEvent(Event &event)
    {
        if (event.type == Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case Key::A:
                {
                    _exit = true;
                    break;
                }
                case Key::B:
                {
                    _cursor = 1;
                    _exit = true;
                    break;
                }
                case Key::DPadLeft:
                {
                    if (_cursor == 1)
                        _cursor = 0;
                    break;
                }
                case Key::DPadRight:
                {
                    if (_cursor == 0 && _dialogType != DialogType::DialogOk)
                        _cursor = 1;
                    break;
                }
                default:
                    break;
            }
        }
    }

    void    MessageBoxImpl::_Draw(void)
    {
        FwkSettings &settings = Preferences::Settings;

        Renderer::SetTarget(TOP);

        // Draw Box backgrounds
        Renderer::DrawRect2(_box, settings.BackgroundMainColor, settings.BackgroundSecondaryColor);

        // Draw Text
        int posY = _box.leftTop.y + _textbox._box.size.y;
        int posX = _box.leftTop.x + 5;
        int maxW = _box.leftTop.x + _box.size.x - 5; ///< Should be -10, but that way a letter that is slighty too big can still be drawn
        int maxH = _box.leftTop.y + _box.size.y - 30;

        /*if (!_title.empty())
        {
            int width = Renderer::DrawSysString(_title.c_str(), posX, posY, maxW, _titleColor);
            Renderer::DrawLine(posX, posY, width - posX + 30, _titleColor);
            posY += 8;
        }
        Renderer::DrawSysStringReturn((const u8 *)_message.c_str(), posX, posY, maxW, settings.MainTextColor, maxH);
        */
        _textbox.Draw();
        // Draw "Buttons"
        //posY += 13;

        // Single button case
        if (_dialogType == DialogType::DialogOk)
        {
            int posX = 160;

            // Background
            Renderer::DrawRect(posX, posY, 80, 20, Color::DimGrey);
            Renderer::DrawRect(posX, posY, 80, 20, Color::Blank, false);

            // Text
            float width = Renderer::GetTextSize("Ok");

            posX += ((80 - width) / 2);
            posY += 2;
            Renderer::DrawSysString("Ok", posX, posY, 380, Color::Blank);
        }
        else
        {
            int posYBak = posY;

            // Ok Button
            {
                int posX = 115;

                // Background
                Renderer::DrawRect(posX, posY, 80, 20, Color::DimGrey);
                Renderer::DrawRect(posX, posY, 80, 20, _cursor ? Color::Grey : Color::Blank, false);

                const char *content = _dialogType == DialogType::DialogOkCancel ? "Ok" : "Yes";

                // Text
                float width = Renderer::GetTextSize(content);

                posX += ((80 - width) / 2);
                posY += 2;
                Renderer::DrawSysString(content, posX, posY, 380, _cursor ? Color::Silver : Color::Blank);
            }

            posY = posYBak;

            // Cancel Button
            {
                int posX = 205;

                // Background
                Renderer::DrawRect(posX, posY, 80, 20, Color::DimGrey);
                Renderer::DrawRect(posX, posY, 80, 20, _cursor ? Color::Blank : Color::Grey, false);

                const char *content = _dialogType == DialogType::DialogOkCancel ? "Cancel" : "No";
                // Text
                float width = Renderer::GetTextSize(content);

                posX += ((80 - width) / 2);
                posY += 2;
                Renderer::DrawSysString(content, posX, posY, 380, _cursor ? Color::Blank : Color::Silver);
            }
        }
    }
}

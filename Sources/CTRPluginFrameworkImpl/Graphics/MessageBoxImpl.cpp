#include "CTRPluginFrameworkImpl/Graphics/MessageBoxImpl.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFrameworkImpl/System/EventManager.hpp"

namespace CTRPluginFramework
{
    MessageBoxImpl::MessageBoxImpl(std::string message, DialogType dType) :
        _message(message), _dialogType(dType), _cursor(0), _exit(false)
    {
        /*
         * line = 16px
         * buttons = 26px
         *  30 + 16 = 46 + 20 = 66
         */

        int lineCount = Renderer::GetLineCount(message.c_str(), 190.f);

        // If text width is inferior to 191px
        if (Renderer::GetTextSize(message.c_str()) < 191.f && lineCount == 1)
        {
            _box = IntRect(100, 82, 200, 75);
        }
        // Text is superior to 180px
        else
        {
            if (lineCount > 3)
                lineCount = 3;

            int height = 16 * lineCount + 59;
            int posY = (240 - height) / 2;
            _box = IntRect(100, posY, 200, height);
        }        
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
            }

            if (_exit)
                break;

            // Draw box
            _Draw();
            Renderer::EndFrame();
        }       

        // Release game if we paused it in this function
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
        Renderer::SetTarget(TOP);

        // Draw Box backgrounds
        Renderer::DrawRect2(_box, Color::Black, Color::BlackGrey);

        // Draw Text
        int posY = _box.leftTop.y + 20;
        Renderer::DrawSysStringReturn((const u8 *)_message.c_str(), _box.leftTop.x + 5, posY, _box.leftTop.x + _box.size.x, Color::Blank, _box.leftTop.y + _box.size.y - 30);
       
        // Draw "Buttons"
        posY += 13;

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

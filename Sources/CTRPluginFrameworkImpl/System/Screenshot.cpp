#include "CTRPluginFrameworkImpl/System/Screenshot.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFrameworkImpl/System/Screen.hpp"
#include "CTRPluginFramework/System/Controller.hpp"
#include "CTRPluginFramework/System/System.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/System/Directory.hpp"

#include "csvc.h"

namespace CTRPluginFramework
{
    #define TIMED 4

    // Static variables
    bool        Screenshot::IsEnabled = false;
    u32         Screenshot::Hotkeys = Key::Start;
    u32         Screenshot::Screens = SCREENSHOT_BOTH;
    std::string Screenshot::Path;// = "/Screenshots";
    std::string Screenshot::Prefix;

    u32         Screenshot::_count;
    u32         Screenshot::_mode;
    u32         Screenshot::_filecount;
    u32         Screenshot::_display = 0;
    Time        Screenshot::_time;
    Clock       Screenshot::_timer;
    Task        Screenshot::_task(Screenshot::TaskCallback);

    LightEvent  Screenshot::_readyEvent;
    LightEvent  Screenshot::_resumeEvent;

    void    Screenshot::Initialize(void)
    {
        LightEvent_Init(&_readyEvent, RESET_STICKY);
        LightEvent_Init(&_resumeEvent, RESET_STICKY);
    }

    bool    Screenshot::OSDCallback(u32 isBottom, void *addr, void *addrB, int stride, int format)
    {
        if (!IsEnabled)
            return IsEnabled;

        if (!_mode && (Controller::GetKeysDown() & Hotkeys) == Hotkeys)
        {
            // Test code !!
            Screens |= TIMED;
            _time = Seconds(30.f);
            _timer.Restart();
            // End test code

            _mode = Screens;
            if (_task.Status() & (Task::Processing | Task::Scheduled))
                _task.Wait();
            _task.Start();
        }

        // If we need to do a screenshot
        if (_mode & SCREENSHOT_BOTH)
        {
            // Top screen handling
            if ((_mode & SCREENSHOT_TOP) && !isBottom)
            {
                // Set screen and backup it (N3DS)
                ScreenImpl::Top->Acquire((u32)addr, (u32)addrB, stride, format, true);

                _mode &= ~SCREENSHOT_TOP;
            }

            if ((_mode & SCREENSHOT_BOTTOM) && isBottom)
            {
                // Set screen and backup it (N3DS)
                ScreenImpl::Bottom->Acquire((u32)addr, (u32)addrB, stride, format, true);

                _mode &= ~SCREENSHOT_BOTTOM;
            }

            // Wake up thread if we're done with all the preparations
            if ((_mode & SCREENSHOT_BOTH) == 0)
            {
                LightEvent_Pulse(&_readyEvent);

                // The screenshot can be async on N3DS
                if (!System::IsNew3DS())
                    LightEvent_Wait(&_resumeEvent);
            }
             ///< Don't execute OSD if we're still waiting for a screen
             return true;
        }

        if (isBottom && _display && _display < 120)
        {
            ScreenImpl::Bottom->Acquire((int)addr, (u32)addrB, stride, format);
            int posY = 0;
            Renderer::SetTarget(BOTTOM);
            Renderer::DrawString((Prefix + Utils::Format(" - %d.bmp", _filecount -1)).c_str(), 0, posY, Color::Blank, Color::Black);
            svcFlushProcessDataCache(Process::GetHandle(), addr, stride * 320);
            ++_display;
        }

        if (isBottom && _display && _display > 130 && _display < 250)
        {
            ScreenImpl::Bottom->Acquire((int)addr, (u32)addrB, stride, format);
            int posY = 0;
            Renderer::SetTarget(BOTTOM);
            Renderer::DrawString("Screenshot: An error occured", 0, posY, Color::Blank, Color::Black);
            ++_display;
        }

        return false;
    }

    s32     Screenshot::TaskCallback(void *arg UNUSED)
    {
        BMPImage        *image = nullptr;
        std::string     name;

        do
        {
            bool error = false;

            // Wait for preparations to be complete
            LightEvent_Wait(&_readyEvent);

            // Create the image
            image = ScreenImpl::Screenshot(Screens & SCREENSHOT_BOTH, image);

            error = !image->IsLoaded();

            // Release the frame
            if (!System::IsNew3DS())
                LightEvent_Pulse(&_resumeEvent);

            if (!error)
            {
                name = Prefix;
                name += (!_filecount ? ".bmp" : Utils::Format(" - %d.bmp", _filecount));
                image->SaveImage(Path + name);
            }

            if (error)
                _display = 131;
            else
            {
                ++_filecount;
                _display = 1;
            }

            // Redo
            if (_mode & TIMED)
                _mode = Screens;

        } while (_mode & TIMED && !_timer.HasTimePassed(_time));

        // Release ressources
        delete image;

        _mode = 0;
        return 0;
    }

    void    Screenshot::UpdateFileCount(void)
    {
        Directory dir(Path);
        std::vector<std::string> files;

        dir.ListFiles(files, ".bmp");

        _filecount = 0;

        std::string name = Prefix + (!_filecount ? ".bmp" : Utils::Format(" - %d.bmp", _filecount));

        for (std::string &filename : files)
        {
            if (filename == name)
            {
                ++_filecount;
                name = Prefix + Utils::Format(" - %d.bmp", _filecount);
            }
        }
    }
}

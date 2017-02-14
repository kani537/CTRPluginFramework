#include "types.h"
#include "ctrulib/services/gspgpu.h"

#include <string>
#include <vector>
#include <cstdio>

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFramework/Graphics.hpp"

#include "CTRPluginFrameworkImpl/Menu.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFramework/System.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

#define SHOWFPS 1

namespace CTRPluginFramework
{
    PluginMenuImpl::PluginMenuImpl(std::string name, std::string note) : 

    _home(new PluginMenuHome(name)),
    _executeLoop(new PluginMenuExecuteLoop()),
    _guide(new GuideReader)
    {
        _isOpen = false;
        _pluginRun = true;
    }

    PluginMenuImpl::~PluginMenuImpl(void)
    {

    }  

    void    PluginMenuImpl::Append(MenuItem *item)
    {
        _home->Append(item);
    }

    /*
    ** Run
    **************/

    int    PluginMenuImpl::Run(void)
    {
        Event                   event;
        EventManager            manager;
        Clock                   clock;
        Clock                   inputClock;
        int                     mode = 0;
        bool                    shouldClose = false;

        // Component
        PluginMenuHome          &home = *_home;
        GuideReader             &guide = *_guide;
        PluginMenuExecuteLoop   &executer = *_executeLoop;

    #if SHOWFPS
        Time            second = Seconds(1);
        Time            frameLimit = second / 30.f;
    #endif

        Time            delta;
        bool            isInit = false;
        OSDImpl         &osd = *(OSDImpl::GetInstance());
        std::vector<Event>     eventList;

        OSD::Notify("Plugin ready !", Color(255, 255, 255), Color());

        // Main loop
        while (_pluginRun)
        {
            // Check Event
            eventList.clear();
            while (manager.PollEvent(event))
            {
                // If Select is pressed
                if (event.type == Event::KeyPressed && event.key.code == Key::Select && inputClock.HasTimePassed(Milliseconds(500)))
                {
                    if (_isOpen)
                    {
                        ProcessImpl::Play();
                        _isOpen = false;
                        isInit = true;
                    }
                    else
                    {
                        Renderer::UseDoubleBuffer(false);
                        ProcessImpl::Pause();
                        _isOpen = true;
                    }
                    inputClock.Restart();   
                }

                if (_isOpen)
                {
                    eventList.push_back(event);
                }
            }
            
            if (_isOpen)
            {   

                if (mode == 0)
                {
                    shouldClose = home(eventList, mode, delta);
                }
                else if (mode == 2)
                {
                    if (guide(eventList, delta))
                        mode = 0;
                }
                
                // FPS of plugin Menu
            #if SHOWFPS
                Color black = Color();
                Color blank = Color(255, 255, 255);
                float   framerate = (second.AsSeconds() / delta.AsSeconds());   
                char buf[40];
                sprintf(buf, "FPS: %03.2f", framerate);
                int posY = 10;
                Renderer::DrawString(buf, 250, posY, blank, black);
            #endif

                // End frame
                Renderer::EndFrame(shouldClose);
                delta = clock.Restart();

                // Close menu
                if (shouldClose)
                {
                    ProcessImpl::Play();
                    _isOpen = false;
                    shouldClose = false;
                }         
            }
            else
            {
                // Execute activate cheat
                executer();
                // Display notifications
                osd();
            }
        }
        return (0);
    }
}

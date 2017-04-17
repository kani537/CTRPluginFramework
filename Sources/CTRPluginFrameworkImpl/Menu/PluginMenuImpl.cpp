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

namespace CTRPluginFramework
{
    PluginMenuImpl::PluginMenuImpl(std::string name, std::string note) : 

    _home(new PluginMenuHome(name)),
    _search(new PluginMenuSearch()),
    _tools(new PluginMenuTools()),
    _executeLoop(new PluginMenuExecuteLoop()),
    _guide(new GuideReader())
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

    void    PluginMenuImpl::Callback(CallbackPointer callback)
    {
        _callbacks.push_back(callback);
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
        PluginMenuTools         &tools = *_tools;
        PluginMenuSearch        &search = *_search;
        GuideReader             &guide = *_guide;
        PluginMenuExecuteLoop   &executer = *_executeLoop;

        Time            delta;
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
                        ProcessImpl::Play(true);
                        _isOpen = false;
                        if (Preferences::InjectBOnMenuClose)
                            Controller::InjectKey(Key::B);
                    }
                    else
                    {
                        ProcessImpl::Pause(true);
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
                { /* Home */
                    shouldClose = home(eventList, mode, delta);
                }
                /*
                else if (mode == 1)
                { /* Mapper *
    
                }
                */
                else if (mode == 2)
                { /* Guide */
                    if (guide(eventList, delta))
                        mode = 0;
                }
                else if (mode == 3)
                { /* Search */
                    if (search(eventList, delta))
                        mode = 0;
                }                
                /*
                else if (mode == 4)
                { /* ActionReplay  *
    
                }
                */
                else if (mode == 5)
                { /* Tools  */
                    if (tools(eventList, delta))
                        mode = 0;
                }

                // End frame
                Renderer::EndFrame(shouldClose);
                delta = clock.Restart();

                // Close menu
                if (shouldClose)
                {
                    ProcessImpl::Play(true);
                    _isOpen = false;
                    shouldClose = false;
                    if (Preferences::InjectBOnMenuClose)
                        Controller::InjectKey(Key::B);
                }    

                if (Controller::IsKeysDown((L + R + Start)))
                {
                    ProcessImpl::Play(true);
                    _pluginRun = false;                    
                    _isOpen = false;     
                }

            }
            else
            {
                // Execute activate cheat
                executer();
                // Execute callbacks
                for (int i = 0; i < _callbacks.size(); i++)
                {
                    _callbacks[i]();
                }
                // Display notifications
                osd();
            }
        }
        return (0);
    }
}

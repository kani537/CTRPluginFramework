#include "Menu.hpp"
#include "Renderer.hpp"
namespace CTRPluginFramework
{
    Menu::Menu(std::string name, std::string note)
    {
        _isOpen = false;
        _folder = new MenuFolder(name, note);
    }

    Menu::~Menu(void)
    {

    }

    void    Menu::Append(MenuItem *item)
    {
        _folder->Append(item);
    }

    int    Menu::Run(void)
    {
        Event           event;
        EventManager    manager;
        Clock           clock;
        Time            delta;
        // Main loop
        while (1)
        {
            // Check Event
            while (manager.PollEvent(event))
            {
                _ProcessEvent(event);
            }
            delta = clock.Restart();
            if (_isOpen)
            {                
                _Update(delta);
                Renderer::StartFrame();
                _RenderBottom();
                _RenderTop();
                Renderer::EndFrame();
            }

        }
        return (0);
    }

    //###########################################
    // Render Top Screen
    //###########################################
    void    Menu::_RenderTop(void)
    {
        Color black = Color();
        Color blank(255, 255, 255);
        int   posY = 35;
        int   posX = 35;
        Renderer::SetTarget(TOP);

        Renderer::DrawRect(30, 30, 320, 200, black);
        Renderer::DrawRect(32, 22, 314, 194, blank, false);

        Renderer::DrawSysString(_folder->name.c_str(), posX, posY, 350, blank);
    }

    //###########################################
    // Render Bottom Screen
    //###########################################
    void    Menu::_RenderBottom(void)
    {
        Color black = Color();
        Color blank(255, 255, 255);

        Renderer::SetTarget(BOTTOM);

        Renderer::DrawRect(20, 20, 280, 200, black);
        Renderer::DrawRect(22, 22, 274, 194, blank, false);
    }

    //###########################################
    // Process Event
    //###########################################
    void    Menu::_ProcessEvent(Event &event)
    {
        static Clock inputClock;

        switch (event.type)
        {
            case Event::KeyPressed:
            {
                // If Select is pressed
                if (event.key.code == Key::Select && inputClock.HasTimePassed(Milliseconds(500)))
                {
                    if (_isOpen)
                    {
                        Process::Play();
                        _isOpen = false;
                    }
                    else
                    {
                        Process::Pause();
                        _isOpen = true;
                    }
                    inputClock.Restart();   
                }
            }
        }
    }

    //###########################################
    // Update menu
    //###########################################
    void    Menu::_Update(Time delta)
    {

    }
}
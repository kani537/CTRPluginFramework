#include "Menu.hpp"
#include "Renderer.hpp"
namespace CTRPluginFramework
{
    Menu::Menu(std::string name, std::string note)
    {
        _isOpen = false;
        _starMode = false;
        _folder = new MenuFolder(name, note);
        _selector = 0;
        _selectedTextSize = 0;
        _scrollOffset = 0.f;
        _maxScrollOffset = 0.f;
        _reverseFlow = false;
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

        _selectedTextSize = Renderer::GetTextSize(_folder->_items[_selector]->name.c_str());
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
        Color lightGrey(190, 190, 190);
        Color silver(160, 160, 160);
        int   posY = 25;
        int   posX = 40;
        Renderer::SetTarget(TOP);

        // Draw background
        Renderer::DrawRect(30, 20, 340, 200, black);
        Renderer::DrawRect(32, 22, 336, 196, blank, false);

        // Draw Title
        int width;
        width = Renderer::DrawSysString(_folder->name.c_str(), posX, posY, 350, blank);
        Renderer::DrawLine(posX, posY, width, blank);
        posY += 7;

        // Draw Entry
        int max = _folder->ItemsCount();
        int i = std::max(0, _selector - 6);//(_selector / 9) * 9;
        max = std::min(max, (i + 8));
        
        for (; i < max; i++)
        {
            MenuItem *item = _folder->_items[i];
            if (i == _selector)
            {
                Renderer::MenuSelector(posX - 5, posY - 3, 320, 20);
            }
            if (item->_type == MenuType::Entry)
            {
                MenuEntry *entry = reinterpret_cast<MenuEntry *>(item);
                Renderer::DrawSysCheckBox(entry->name.c_str(), posX, posY, 350, i == _selector ? blank : silver, entry->IsActivated(), _scrollOffset);  
            }
            else
            {
                Renderer::DrawSysFolder(item->name.c_str(), posX, posY, 350, i == _selector ? blank : silver, _scrollOffset);
            }
            posY += 4;
        }

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
        Renderer::DrawRect(22, 22, 276, 196, blank, false);
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
                // Moving Selector
                if (event.key.code == Key::DPadUp)
                {
                    if (_selector == 0)
                        _selector = _folder->ItemsCount() - 1;
                    else
                        _selector--;
                }
                if (event.key.code == Key::DPadDown)
                {
                    if (_selector == _folder->ItemsCount() - 1)
                        _selector = 0;
                    else
                        _selector++;
                }
                // Triggering entry
                if (event.key.code == Key::A)
                {
                    if (_folder->_items[_selector]->_type == MenuType::Entry)
                    {
                        reinterpret_cast<MenuEntry *>(_folder->_items[_selector])->_TriggerState();
                    }
                    else
                    {
                        MenuFolder *p = reinterpret_cast<MenuFolder *>(_folder->_items[_selector]);
                        p->_Open(_folder);
                        _folder = p;
                        _selector = 0;
                    }
                }

                // Close Folder
                if (event.key.code == Key::B)
                {
                    MenuFolder *p = _folder->_Close();
                    if (p != nullptr)
                    {
                        _selector = 0;
                        _folder = p;
                    }
                }
                // Update selected text size
                _selectedTextSize = Renderer::GetTextSize(_folder->_items[_selector]->name.c_str());
                _maxScrollOffset = (float)_selectedTextSize - 300.f;
                _scrollClock.Restart();
                _scrollOffset = 0.f;
                _reverseFlow = false;
            }
        }
    }

    //###########################################
    // Update menu
    //###########################################
    void    Menu::_Update(Time delta)
    {
        if (_selectedTextSize > 350 && _scrollClock.HasTimePassed(Seconds(2)))
        {
            if (!_reverseFlow && _scrollOffset < _maxScrollOffset)
            {
                _scrollOffset += 25.f * delta.AsSeconds();
            }
            else
            {
                _scrollOffset -= 38.f * delta.AsSeconds();
                if (_scrollOffset <= 0.0f)
                {
                    _reverseFlow = false;
                    _scrollOffset = 0.f;
                }
                else
                    _reverseFlow = true;
            }
        }
    }
}
#include "Menu.hpp"
#include "Renderer.hpp"
#include "Touch.hpp"
#include "ctrulib/services/gspgpu.h"
#include "ctrulib/gfx.h"

namespace CTRPluginFramework
{
    Menu::Menu(std::string name, std::string note) : _startLine(-1, -1), _endLine(-1, -1)
    {
        _isOpen = false;
        _starMode = false;
        _folder = new MenuFolder(name, note);
        _selector = 0;
        _selectedTextSize = 0;
        _scrollOffset = 0.f;
        _maxScrollOffset = 0.f;
        _reverseFlow = false;
        _arcX = 50;
        _arcRadius = 1;
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
        Clock           inputClock;
        Time            second = Seconds(1);
        Time            frameLimit = second / 30.f;
        float           framerate;
        Time            delta;

        _selectedTextSize = Renderer::GetTextSize(_folder->_items[_selector]->name.c_str());
        // Main loop
        while (1)
        {
            // Check Event
            while (manager.PollEvent(event))
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
                if (_isOpen)
                    _ProcessEvent(event);
            }
            
            if (_isOpen)
            {   
                delta = clock.Restart();
                framerate = (second.AsSeconds() / delta.AsSeconds());         
                _Update(delta);

                Renderer::StartFrame();
                _RenderBottom();
                _RenderTop();
                char buf[40];
                sprintf(buf, "FPS: %03.2f", framerate);
                Color blank(255, 255, 255);
                Color black = Color();
                int posY = 10;
                Renderer::DrawString(buf, 320, posY, blank, black);
                Renderer::EndFrame();
                while(clock.GetElapsedTime() < frameLimit);

            }
            else
            {
                for (int i = 0; i < _executeLoop.size(); i++)
                {
                    MenuEntry *entry = _executeLoop[i];
                    if (entry)
                    {
                        if (entry->_Execute())
                        {
                            _executeLoop[i] = nullptr;
                            entry->_executeIndex = -1;
                            _freeIndex.push(i);                    
                        }
                    }
                }
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
                Renderer::DrawSysCheckBox(entry->name.c_str(), posX, posY, 350, i == _selector ? blank : silver, entry->IsActivated(), i == _selector ? _scrollOffset : 0.f);  
            }
            else
            {
                Renderer::DrawSysFolder(item->name.c_str(), posX, posY, 350, i == _selector ? blank : silver, i == _selector ? _scrollOffset : 0.f);
            }
            posY += 4;
        }

    }

    float _a = 5.f;
    float _b = 1.f;
    int _max = 0;
    int _aff = 0;
    Color _c;
    extern "C" void    gfxReadFramebufferInfo(gfxScreen_t screen, GSPGPU_FramebufferInfo *out);
    //###########################################
    // Render Bottom Screen
    //###########################################
    void    Menu::_RenderBottom(void)
    {
        Color black = Color();
        Color blank(255, 255, 255);
        Color green(0, 255, 0);
        Color blue(0, 255, 255);
        GSPGPU_CaptureInfo infos = {0};
        GSPGPU_FramebufferInfo inf = {0};

        Renderer::SetTarget(BOTTOM);

        Renderer::DrawRect(20, 20, 280, 200, black);
        Renderer::DrawRect(22, 22, 276, 196, blank, false);
        int posY = 35;
        Renderer::DrawString("CTRPluginFramework", 1000, posY, blank);
        posY = Screen::Top->Debug(25, posY);
        //GSPGPU_ImportDisplayCaptureInfo(&infos);
        //gfxReadFramebufferInfo(GFX_TOP, &inf);
        char buf[100];
        //sprintf(buf, "active: %d, 0: %08X, 1:%08X, dspSelect: %d", 
        //    inf.active_framebuf, inf.framebuf0_vaddr, inf.framebuf1_vaddr, inf.framebuf_dispselect);
       // Renderer::DrawString(buf, 25, posY, blank);
        if (_startLine.x != -1)
        {
           
            Renderer::Line(_startLine, _endLine, green);
            //Renderer::Arc(_endLine.x, _endLine.y, _arcRadius, green);
            //Renderer::EllipseIncomplete(_endLine.x, _endLine.y, _a, _b, _max, _aff, blue);
            //char buf[100];
            sprintf(buf, "%f", _a);
            Renderer::DrawString(buf, 25, posY, blank);
            IntRect rect(_startLine, _endLine - _startLine);
            Renderer::RoundedRectangle(rect, _a, 50, _c);
        }
        
        
    }

    //###########################################
    // Process Event
    //###########################################
    void    Menu::_ProcessEvent(Event &event)
    {
        

        switch (event.type)
        {
            case Event::TouchBegan:
            {
                _startLine.x = event.touch.x;
                _startLine.y = event.touch.y;
                _endLine.x = event.touch.x;
                _endLine.y = event.touch.y;
                 u64 tick = svcGetSystemTick();
                _c = Color(tick >> 16 & 0xFF, tick >> 8 & 0xFF, tick & 0xFF);
                break;
            }
            case Event::TouchMoved:
            {
                _endLine.x = event.touch.x;
                _endLine.y = event.touch.y;
                break;
            }
            case Event::KeyPressed:
            {
                // Changing Arc
                if (event.key.code == Key::DPadRight)
                    _a++;
                else if (event.key.code == Key::DPadLeft)
                    _a--;
                else if (event.key.code == Key::L)
                    _b--;
                else if (event.key.code == Key::R)
                    _b++;
                else if (event.key.code == Key::X)
                    _max++;
                else if (event.key.code == Key::Y)
                    _max--;
                else if (event.key.code == Key::ZL)
                    _aff--;
                else if (event.key.code == Key::ZR)
                    _aff++;
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
                    // If the selected element is an entry
                    if (_folder->_items[_selector]->_type == MenuType::Entry)
                    {
                        MenuEntry *entry = reinterpret_cast<MenuEntry *>(_folder->_items[_selector]);
                        // Change the state
                        bool state = entry->_TriggerState();
                        // If the entry has a valid funcpointer
                        if (entry->GameFunc != nullptr)
                        {
                            // If is activated add to executeLoop
                            if (state)
                            {
                                // Check for free index in the vector
                                if (!_freeIndex.empty())
                                {
                                    int i = _freeIndex.front();
                                    _freeIndex.pop();
                                    _executeLoop[i] = entry;
                                    entry->_executeIndex = i;
                                }
                                // Else add it at the back
                                else
                                {
                                    int i = _executeLoop.size();
                                    _executeLoop.push_back(entry);
                                    entry->_executeIndex = i;
                                }
                            }
                            // Else removes it
                            else
                            {
                                int i = entry->_executeIndex;
                                if (i >= 0 && i < _executeLoop.size())
                                {
                                    _executeLoop[i] = nullptr;
                                    _freeIndex.push(i);
                                }
                                entry->_executeIndex = -1;
                            }
                        }                      
                    }
                    // Else the selected element is a folder
                    else
                    {
                        MenuFolder *p = reinterpret_cast<MenuFolder *>(_folder->_items[_selector]);
                        p->_Open(_folder, _selector);
                        _folder = p;
                        _selector = 0;
                    }
                }

                // Close Folder
                if (event.key.code == Key::B)
                {
                    MenuFolder *p = _folder->_Close(_selector);
                    if (p != nullptr)
                    {
                        _folder = p;
                    }
                }
                // Update selected text size
                _selectedTextSize = Renderer::GetTextSize(_folder->_items[_selector]->name.c_str());
                _maxScrollOffset = (float)_selectedTextSize - 200.f;
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
        if (_selectedTextSize >= 280 && _scrollClock.HasTimePassed(Seconds(2)))
        {
            if (!_reverseFlow && _scrollOffset < _maxScrollOffset)
            {
                _scrollOffset += 29.f * delta.AsSeconds();
            }
            else
            {
                _scrollOffset -= 55.f * delta.AsSeconds();
                if (_scrollOffset <= 0.0f)
                {
                    _reverseFlow = false;
                    _scrollOffset = 0.f;
                    _scrollClock.Restart();
                }
                else
                    _reverseFlow = true;
            }
        }
    }
}
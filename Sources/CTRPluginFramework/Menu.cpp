#include "types.h"
#include "ctrulib/services/gspgpu.h"

#include <string>
#include <vector>
#include <cstdio>

#include "CTRPluginFramework/Vector.hpp"
#include "CTRPluginFramework/Rect.hpp"
#include "CTRPluginFramework/Line.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/Graphics/Renderer.hpp"
#include "CTRPluginFramework/Menu.hpp"
#include "CTRPluginFramework/MenuFolder.hpp"
#include "CTRPluginFramework/MenuEntry.hpp"
#include "CTRPluginFramework/Controller.hpp"
#include "CTRPluginFramework/Touch.hpp"
#include "CTRPluginFramework/Events.hpp"
#include "CTRPluginFramework/EventManager.hpp"
#include "CTRPluginFramework/Time.hpp"
#include "CTRPluginFramework/Clock.hpp"
#include "CTRPluginFramework/Process.hpp"
#include "CTRPluginFramework/Graphics/Icon.hpp"



namespace CTRPluginFramework
{
    static MenuItem m(MenuType::Entry);

    bool    _shouldClose = false;
    Menu::Menu(std::string name, std::string note) : 
    _startLine(-1, -1), _endLine(-1, -1),
    _showStarredBtn("Favorite", *this, &Menu::_StarMode, IntRect(30, 70, 120, 30), Icon::DrawFavorite), 
    _gameGuideBtn("Game Guide", *this, &Menu::Null, IntRect(30, 105, 120, 30), Icon::DrawGuide),    
    _toolsBtn("Tools", *this, &Menu::Null, IntRect(30, 140, 120, 30), Icon::DrawTools),
    _hidMapperBtn("Mapper", *this, &Menu::Null, IntRect(165, 70, 120, 30), Icon::DrawController),
    _searchBtn("Search", *this, &Menu::Null, IntRect(165, 105, 120, 30), Icon::DrawSearch),
    _AddFavoriteBtn(*this, &Menu::_StarItem, IntRect(50, 30, 25, 25), Icon::DrawAddFavorite),
    _InfoBtn(*this, &Menu::Null, IntRect(90, 30, 25, 25), Icon::DrawInfo, false)
    {
        _isOpen = false;
        _starMode = false;
        _folder = new MenuFolder(name, note);
        _starred = new MenuFolder("Favorites");
        _selector = 0;
        _selectedTextSize = 0;
        _scrollOffset = 0.f;
        _maxScrollOffset = 0.f;
        _reverseFlow = false;
    }

    Menu::~Menu(void)
    {

    }
    void    Menu::Null(void)
    {

    } 
    void    Menu::_StarMode(void)
    {
        static int selector = 0;

        if (_starMode)
        {
            _selector = selector;
            _starMode = false;
        }
        else
        {
            selector = _selector;
            _selector = 0;
            _starMode = true;
        }
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
        Color           blank(255, 0, 255);
        bool            isInit = false;

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
                        isInit = true;
                    }
                    else
                    {
                        Renderer::UseDoubleBuffer(false);
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
                
              /*  char buf[40];
                sprintf(buf, "FPS: %03.2f", framerate);
                Color blank(255, 255, 255);
                Color black = Color();
                int posY = 10;
                Renderer::DrawString(buf, 320, posY, blank, black);*/
                Renderer::EndFrame(_shouldClose);
                if (_shouldClose)
                {
                    Process::Play();
                    _isOpen = false;
                    _shouldClose = false;
                }

                _gameGuideBtn();
                _showStarredBtn();
                _toolsBtn();
                _hidMapperBtn();
                _searchBtn();

                _AddFavoriteBtn();
                _InfoBtn();
               // while(clock.GetElapsedTime() < frameLimit);
            
            }
            else
            {
                        // Draw Touch Cursor
                if (isInit && Touch::IsDown())
                {
                    UIntVector t(Touch::GetPosition());
                    int posX = t.x - 2;
                    int posY = t.y - 1;
                    Renderer::UseDoubleBuffer(true);
                    Renderer::SetTarget(BOTTOM);
                    Renderer::DrawSysString("\uE058", posX, posY, 320, blank);
                    gspWaitForVBlank();
                } 
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
        static Color black = Color();
        static Color blank(255, 255, 255);
        static Color lightGrey(190, 190, 190);
        static Color dimGrey(15, 15, 15);
        static Color silver(160, 160, 160);
        static IntRect background(30, 20, 340, 200);

        int   posY = 25;
        int   posX = 40;
        Renderer::SetTarget(TOP);

        // Draw background
        Renderer::DrawRect2(background, black, dimGrey);
        Renderer::DrawRect(32, 22, 336, 196, blank, false);

        MenuFolder *folder = _starMode ? _starred : _folder;

        // Draw Title
        int width;
        width = Renderer::DrawSysString(folder->name.c_str(), posX, posY, 350, blank);
        Renderer::DrawLine(posX, posY, width, blank);
        posY += 7;

        // Draw Entry
        int max = folder->ItemsCount();
        if (max == 0)
            return;
        int i = std::max(0, _selector - 6);//(_selector / 9) * 9;
        max = std::min(max, (i + 8));
        
        for (; i < max; i++)
        {
            MenuItem *item = folder->_items[i];
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

    //###########################################
    // Render Bottom Screen
    //###########################################
    void    Menu::_RenderBottom(void)
    {
        static Color black = Color();
        static Color blank(255, 255, 255);
        static Color green(0, 255, 0);
        static Color blue(0, 255, 255);
        static Color lightGrey(190, 190, 190);
        static Color dimGrey(15, 15, 15);
        static Color silver(160, 160, 160);
        static IntRect background(20, 20, 280, 200);
        static IntRect closeIcon(275, 24, 20, 20);

        Renderer::SetTarget(BOTTOM);

        // Background
        Renderer::DrawRect2(background, black, dimGrey);
        Renderer::DrawRect(22, 22, 276, 196, blank, false);


        int posY = 205;
        Renderer::DrawString("CTRPluginFramework", 100, posY, blank);

        posY = 35;

        bool isTouchDown = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());

        bool closeIsTouch = closeIcon.Contains(touchPos);
        Icon::DrawClose(275, 24, closeIsTouch);


        _showStarredBtn.Draw();
        _gameGuideBtn.Draw();
        _toolsBtn.Draw();
        _hidMapperBtn.Draw();
        _searchBtn.Draw();

        _AddFavoriteBtn.Draw();
        _InfoBtn.Draw();

        // Draw Touch Cursor
        if (isTouchDown && background.Contains(touchPos))
        {
            int posX = touchPos.x - 2;
            int posY = touchPos.y - 1;
            touchPos.x += 10;
            touchPos.y += 15;
            if (background.Contains(touchPos))
                Renderer::DrawSysString("\uE058", posX, posY, 320, blank);
        } 
        static bool _closing = false;
        if (closeIsTouch)
        {
            _closing = true;
        }   
        else if (_closing)
        {
            _closing = false;
            _shouldClose = true;
        }
    }

    //###########################################
    // Process Event
    //###########################################
    void    Menu::_ProcessEvent(Event &event)
    {
        static Clock fastScroll;
        static Clock inputClock;

        MenuFolder *folder = _starMode ? _starred : _folder;

        switch (event.type)
        {
            case Event::KeyDown:
            {
                if (fastScroll.HasTimePassed(Seconds(0.5f)) && inputClock.HasTimePassed(Milliseconds(400)))
                switch (event.key.code)
                {
                    /*
                    ** Selector
                    **************/
                    case Key::DPadUp:
                    {
                        if (_selector == 0)
                            _selector = folder->ItemsCount() - 1;
                        else
                            _selector--;
                        break;
                    }
                    case Key::DPadDown:
                    {
                        if (_selector == folder->ItemsCount() - 1)
                            _selector = 0;
                        else
                            _selector++;
                        break;
                    }
                    inputClock.Restart();                  
                }
                break;
            } // Event::KeyDown
            case Event::KeyPressed:
            {
                switch (event.key.code)
                {
                    /*
                    ** Selector
                    **************/
                    case Key::DPadUp:
                    {
                        if (_selector == 0)
                            _selector = folder->ItemsCount() - 1;
                        else
                            _selector--;
                        fastScroll.Restart();    
                        break;
                    }
                    case Key::DPadDown:
                    {
                        if (_selector == folder->ItemsCount() - 1)
                            _selector = 0;
                        else
                            _selector++;
                        fastScroll.Restart();
                        break;
                    }
                    /*
                    ** Trigger entry
                    ** Top Screen
                    ******************/
                    case Key::A:
                    {
                        _TriggerEntry();
                        break;
                    }
                    /*
                    ** Closing Folder
                    ********************/
                    case Key::B:
                    {
                        MenuFolder *p = folder->_Close(_selector, _starMode);
                        if (p != nullptr)
                        {
                            if (_starMode)
                                _starred = p;
                            else
                                _folder = p;
                        }
                        break;
                    }

                } 
                break;
            } // End Key::Pressed event

            /*
            ** Scrolling text variables
            *********************************/
            if (event.key.code != Key::Touchpad)
            {
                _selectedTextSize = folder->ItemsCount() > 0 ? Renderer::GetTextSize(folder->_items[_selector]->name.c_str()) : 0;
                _maxScrollOffset = (float)_selectedTextSize - 200.f;
                _scrollClock.Restart();
                _scrollOffset = 0.f;
                _reverseFlow = false;               
            }
        } // End switch

        /*
        ** Update favorite state
        **************************/
        if (folder->ItemsCount() > 0)
        {
            MenuItem *item = folder->_items[_selector];
            _AddFavoriteBtn.SetState(item->_IsStarred());
            _InfoBtn.Enable(item->note.size() > 0);
        }
    }

    //###########################################
    // Update menu
    //###########################################
    void    Menu::_Update(Time delta)
    {
        /*
        ** Scrolling
        *************/

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

        /*
        ** Buttons
        *************/
        bool isTouched = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());

        _showStarredBtn.Update(isTouched, touchPos);
        _gameGuideBtn.Update(isTouched, touchPos);
        _toolsBtn.Update(isTouched, touchPos);
        _hidMapperBtn.Update(isTouched, touchPos);
        _searchBtn.Update(isTouched, touchPos);
        _InfoBtn.Update(isTouched, touchPos);
        _AddFavoriteBtn.Update(isTouched, touchPos);

    }

    void    Menu::_TriggerEntry(void)
    {
        MenuFolder *folder = _starMode ? _starred : _folder;
        /*
        ** MenuEntry
        **************/
        if (_selector >= folder->ItemsCount())
            return;
        if (folder->_items[_selector]->_type == MenuType::Entry)
        {
            MenuEntry *entry = reinterpret_cast<MenuEntry *>(folder->_items[_selector]);
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
        /*
        ** MenuFolder
        ****************/
        else
        {
            MenuFolder *p = reinterpret_cast<MenuFolder *>(folder->_items[_selector]);
            p->_Open(folder, _selector, _starMode);
            if (_starMode)
                _starred = p;
            else
                _folder = p;
            _selector = 0;
        }
    }

    void   Menu::_StarItem(void)
    {
        MenuFolder *folder = _starMode ? _starred : _folder;

        if (_selector >= folder->ItemsCount())
            return;
        
        MenuItem *item = folder->_items[_selector];

        if (item)
        {
            bool star = item->_TriggerStar();

            if (star)
                _starred->Append(item);
            else
            {
                for (int i = 0; i < folder->ItemsCount(); i++)
                {
                    MenuItem *it = folder->_items[i];

                    if (it == item)
                    {
                        _starred->_items.erase(_starred->_items.begin() + i);
                        break;
                    }
                }
            }
        }
    }
}
#include "CTRPluginFrameworkImpl/Menu/PluginMenuHome.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"

namespace CTRPluginFramework
{
    PluginMenuHome::PluginMenuHome(std::string name) :
        _showStarredBtn("Favorite", *this, nullptr, IntRect(30, 70, 120, 30), 7, Icon::DrawFavorite),
        _hidMapperBtn("Mapper", *this, nullptr, IntRect(165, 70, 120, 30), 7, Icon::DrawController),
        _gameGuideBtn("Game Guide", *this, nullptr, IntRect(30, 105, 120, 30), 7, Icon::DrawGuide),        
        _searchBtn("Search", *this, nullptr, IntRect(165, 105, 120, 30), 7, Icon::DrawSearch),  
        _arBtn("ActionReplay", *this, nullptr, IntRect(30, 140, 120, 30), 7),
        _toolsBtn("Tools", *this, nullptr, IntRect(165, 140, 120, 30), 7, Icon::DrawTools),

        _AddFavoriteBtn(*this, &PluginMenuHome::_StarItem, IntRect(50, 30, 25, 25), Icon::DrawAddFavorite),
        _InfoBtn(*this, &PluginMenuHome::_DisplayNote, IntRect(90, 30, 25, 25), Icon::DrawInfo, false),

        _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose),
        _keyboardBtn(*this, nullptr, IntRect(130, 30, 25, 25), Icon::DrawKeyboard, false)
    {
        _folder = new MenuFolderImpl(name);
        _starred = new MenuFolderImpl("Favorites");
        _starredConst = _starred;

        _starMode = false;
        _selector = 0;
        _selectedTextSize = 0;
        _scrollOffset = 0.f;
        _maxScrollOffset = 0.f;
        _reverseFlow = false;
        _noteTB = nullptr;
    }

    bool    PluginMenuHome::operator()(EventList &eventList, int &mode, Time &delta)
    {
        static int note = 0;

        // Process events
        if (note && !_noteTB)
            note = 0;
        else if (note)
        {
            for (int i = 0; i < eventList.size(); i++)
                if (_noteTB->ProcessEvent(eventList[i]) == false)
                {
                    note = 0;
                     _InfoBtn.SetState(false);
                    break;
                }        
        }
        else
        {
            for (int i = 0; i < eventList.size(); i++)
                _ProcessEvent(eventList[i]);            
        }


        // Update UI
        _Update(delta);

        // Render top
        Renderer::SetTarget(TOP);
        if (note)
            _noteTB->Draw();
        else
            _RenderTop();

        // RenderBottom
        _RenderBottom();

        if (_showStarredBtn())
        {
            static int bak = 0;
            std::swap(bak, _selector);
            _starMode = !_starMode;

            MenuFolderImpl *f = _starMode ? _starred : _folder;
            if (f->ItemsCount() == 0)
            {
                _InfoBtn.Enable(false);
                _AddFavoriteBtn.Enable(false);
            }
            else
            {
                MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl *>(f->_items[_selector]);
                _InfoBtn.Enable(e->note.size() > 0);
                _AddFavoriteBtn.Enable(true);
                _AddFavoriteBtn.SetState(e->_IsStarred());
            }

        }

        /*
        else if (_hidMapperBtn())
            mode = 1;
        */

        else if (_gameGuideBtn())
            mode = 2;
        else if (_searchBtn())
            mode = 3;
        /*
        else if (_arBtn())
            mode = 4;
        */

        else if (_toolsBtn())
            mode = 5;

        if (_InfoBtn())
            note = !note;

        _AddFavoriteBtn();

        if (_keyboardBtn())
        {
            MenuFolderImpl *f = _starMode ? _starred : _folder;
            MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl *>(f->_items[_selector]);
            e->MenuFunc(e->_owner);
        }

        return (_closeBtn());
    }

    void    PluginMenuHome::Append(MenuItem *item)
    {
        _folder->Append(item);
    }

    //###########################################
    // Process Event
    //###########################################
    void    PluginMenuHome::_ProcessEvent(Event &event)
    {
        static Clock fastScroll;
        static Clock inputClock;
        static MenuItem *last = nullptr;

        MenuFolderImpl *folder = _starMode ? _starred : _folder;
        MenuItem *item;

        switch (event.type)
        {
            case Event::KeyDown:
            {
                if (fastScroll.HasTimePassed(Milliseconds(800)) && inputClock.HasTimePassed(Milliseconds(100)))
                {
                    switch (event.key.code)
                    {
                        /*
                        ** Selector
                        **************/
                        case Key::CPadUp:
                        case Key::DPadUp:
                        {
                            if (_selector > 0)
                                _selector--;
                            else
                                _selector = std::max((int)folder->ItemsCount() - 1, 0);
                            break;
                        }
                        case Key::CPadDown:
                        case Key::DPadDown:
                        {
                            if (_selector < folder->ItemsCount() - 1)
                                _selector++;
                            else
                                _selector = 0;
                            break;
                        } 
                        case Key::CPadLeft:
                        case Key::DPadLeft:
                        {
                            if (_selector > 0)
                                _selector = std::max(0, (int)(_selector - 4));
                            break;
                        }
                        case Key::CPadRight:
                        case Key::DPadRight:
                        {
                            if (_selector < folder->ItemsCount() - 1)
                                _selector = std::min((int)(folder->ItemsCount() - 1), (int)(_selector + 4));
                            break;
                        } 
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
                    case Key::CPadUp:
                    case Key::DPadUp:
                    {
                        if (_selector > 0)
                            _selector--;
                        else
                            _selector = std::max((int)folder->ItemsCount() - 1, 0);
                        fastScroll.Restart();
                        break;
                    }
                    case Key::CPadDown:
                    case Key::DPadDown:
                    {
                        if (_selector < folder->ItemsCount() - 1)
                            _selector++;
                        else
                            _selector = 0;
                        fastScroll.Restart();
                        break;
                    }
                    case Key::CPadLeft:
                    case Key::DPadLeft:
                    {
                        if (_selector > 0)
                            _selector = std::max(0, (int)(_selector - 4));
                        fastScroll.Restart();
                        break;
                    }
                    case Key::CPadRight:
                    case Key::DPadRight:
                    {
                        if (_selector < folder->ItemsCount() - 1)
                            _selector = std::min((int)(folder->ItemsCount() - 1), (int)(_selector + 4));
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
                        MenuFolderImpl *p = folder->_Close(_selector, _starMode);
                        if (p != nullptr)
                        {
                            if (_starMode)
                            {
                                _starred = p;
                                folder = p;
                            }
                            else
                            {
                                _folder = p;
                                folder = p;
                            }
                        }
                        break;
                    }

                } 
                break;
            } // End Key::Pressed event       


        } // End switch

        folder = _starMode ? _starred : _folder;
        /*
        ** Scrolling text variables
        *********************************/
        if (folder->ItemsCount() > 0 && event.key.code != Key::Touchpad && (event.type < Event::TouchBegan || event.type > Event::TouchSwipped))
        {          
            item = folder->_items[_selector];
            _selectedTextSize = folder->ItemsCount() > 0 ? Renderer::GetTextSize(item->name.c_str()) : 0;
            _maxScrollOffset = (float)_selectedTextSize - 200.f;
            _scrollClock.Restart();
            _scrollOffset = 0.f;
            _reverseFlow = false;      
        }
        /*
        ** Update favorite state
        **************************/

        if (folder->ItemsCount() > 0  && _selector < folder->ItemsCount())
        {
            item = folder->_items[_selector];
            _AddFavoriteBtn.SetState(item->_IsStarred());
            
            if (last != item)
            {
                if (item->_type == MenuType::Entry)
                {
                    MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl *>(item);
                    _keyboardBtn.Enable(e->MenuFunc != nullptr);
                }
                else
                    _keyboardBtn.Enable(false);
                 
                last = item;
                if (_noteTB != nullptr)
                {
                    delete _noteTB;
                    _noteTB = nullptr;
                }
                
                if (item->note.size() > 0)
                {
                    static IntRect noteRect(40, 30, 320, 180);
                    _noteTB = new TextBox(item->name, item->note, noteRect);
                    _InfoBtn.Enable(true);
                }
                else
                    _InfoBtn.Enable(false);
            }
        }
    }

    //###########################################
    // Render Menu
    //###########################################

    void    PluginMenuHome::_RenderTop(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static Color    silver(160, 160, 160);
        static IntRect  background(30, 20, 340, 200);

        int   posY = 25;
        int   posX = 40;

        // Draw background
        if (Preferences::topBackgroundImage->IsLoaded())
            Preferences::topBackgroundImage->Draw(background.leftTop);
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            Renderer::DrawRect(32, 22, 336, 196, blank, false);            
        }

        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        // Draw Title
        int width;
        width = Renderer::DrawSysString(folder->name.c_str(), posX, posY, 350, blank);
        Renderer::DrawLine(posX, posY, width, blank);
        posY += 7;

        // Draw Entry
        int max = folder->ItemsCount();
        if (max == 0)
            return;
        int i = std::max(0, _selector - 6);
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
                MenuEntryImpl *entry = reinterpret_cast<MenuEntryImpl *>(item);
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
    void    PluginMenuHome::_RenderBottom(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static IntRect  background(20, 20, 280, 200);
        static Clock    creditClock;
        static bool     framework = true;

        Renderer::SetTarget(BOTTOM);

        // Background
        if (Preferences::bottomBackgroundImage->IsLoaded())
            Preferences::bottomBackgroundImage->Draw(background.leftTop);
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            Renderer::DrawRect(22, 22, 276, 196, blank, false);            
        }


        int posY = 205;
        if (framework)
            Renderer::DrawString((char *)"CTRPluginFramework", 100, posY, blank);
        else
            Renderer::DrawString((char *)"by Nanquitas", 124, posY, blank);

        if (creditClock.HasTimePassed(Seconds(5)))
        {
            creditClock.Restart();
            framework = !framework;
        }

        posY = 35;

        // Draw buttons
        _showStarredBtn.Draw();
        _gameGuideBtn.Draw();
        _arBtn.Draw();
        _toolsBtn.Draw();
        _hidMapperBtn.Draw();
        _searchBtn.Draw();

        _AddFavoriteBtn.Draw();
        _InfoBtn.Draw();
        _keyboardBtn.Draw();

        _closeBtn.Draw();
    }

    //###########################################
    // Update menu
    //###########################################
    void    PluginMenuHome::_Update(Time delta)
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
        bool        isTouched = Touch::IsDown();
        IntVector   touchPos(Touch::GetPosition());

        _showStarredBtn.Update(isTouched, touchPos);
        _gameGuideBtn.Update(isTouched, touchPos);
        _arBtn.Update(isTouched, touchPos);
        _toolsBtn.Update(isTouched, touchPos);
        _hidMapperBtn.Update(isTouched, touchPos);
        _searchBtn.Update(isTouched, touchPos);

        _InfoBtn.Update(isTouched, touchPos);
        _AddFavoriteBtn.Update(isTouched, touchPos);
        _keyboardBtn.Update(isTouched, touchPos);
        _closeBtn.Update(isTouched, touchPos);
    }

    void    PluginMenuHome::_TriggerEntry(void)
    {
        MenuFolderImpl *folder = _starMode ? _starred : _folder;


        if (_selector >= folder->ItemsCount())
            return;

        MenuItem    *item = folder->_items[_selector];

        /*
        ** MenuEntryImpl
        **************/
        if (item->_type == MenuType::Entry)
        {
            MenuEntryImpl *entry = reinterpret_cast<MenuEntryImpl *>(item);
            // Change the state
            bool just = entry->_flags.justChanged;
            bool state = entry->_TriggerState();
            // If the entry has a valid funcpointer
            if (entry->GameFunc != nullptr)
            {
                // If is activated add to executeLoop
                if (state)
                {
                    PluginMenuExecuteLoop::Add(entry);
                }
                else if (just)
                {
                    PluginMenuExecuteLoop::Remove(entry);
                }
            }
        }
        /*
        ** MenuFolderImpl
        ****************/
        else
        {
            MenuFolderImpl *p = reinterpret_cast<MenuFolderImpl *>(item);
            p->_Open(folder, _selector, _starMode);
            if (_starMode)
                _starred = p;
            else
                _folder = p;
            _selector = 0;
        }
    }


    void    PluginMenuHome::_DisplayNote(void)
    {
        if (_noteTB != nullptr)
        {
            if (_noteTB->IsOpen())
                _noteTB->Close();
            else
                _noteTB->Open();
        }
    }

    void   PluginMenuHome::_StarItem(void)
    {
        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        if (_selector >= folder->ItemsCount())
            return;

        MenuItem *item = folder->_items[_selector];

        if (item)
        {
            bool star = item->_TriggerStar();

            if (star)
                _starredConst->Append(item);
            else
            {
                int count = _starredConst->ItemsCount();

                if (count == 1)
                {
                    _starredConst->_items.clear();
                }
                else
                {
                    for (int i = 0; i < count; i++)
                    {
                        MenuItem *it = _starredConst->_items[i];

                        if (it == item)
                        {
                            _starredConst->_items.erase(_starredConst->_items.begin() + i);
                            break;
                        }
                    }                    
                }
                if (_selector >= folder->ItemsCount())
                    _selector = std::max((int)folder->ItemsCount() - 1, 0);
            }
        }
    }
}
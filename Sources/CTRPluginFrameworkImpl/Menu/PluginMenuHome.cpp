#include "CTRPluginFrameworkImpl/Menu/PluginMenuHome.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"

namespace CTRPluginFramework
{
    static char* g_ctrpfString = nullptr;
    static char* g_bymeString = nullptr;

    static const u32 g_ctrpf[18] =
    {
        0x00000043, 0x00000054, 0x00000148, 0x00000140, 0x0000006C, 0x00000075, 0x0000019C, 0x000001A4, 0x00006E00, 0x00004600, 0x0001C800, 0x00018400, 0x00006D00, 0x00006500, 0x0001DC00, 0x0001BC00, 0x00000072, 0x0000006B,
    };

    static const u32 g_byme[12] =
    {
        0x00000062, 0x00000079, 0x00000080, 0x00000138, 0x00000061, 0x0000006E, 0x000001C4, 0x000001D4, 0x00006900, 0x00007400, 0x00018400, 0x0001CC00,
    };

    /*
    void    encoder(u32 *out, char *in)
    {
    int i = 0;
    while (*in)
    {
    u32 c = (u32)*in++;

    c = (c << (i++ & 0b1010));

    *out++ = c;
    }
    }*/

    void decoder(char* out, const u32* in, int size)
    {
        int i = 0;
        while (size)
        {
            u32 c = *in++;

            c = (c >> (i++ & 0b1010));

            *out++ = (char)c;

            size--;
        }
        *out = '\0';
    }

    PluginMenuHome::PluginMenuHome(std::string &name) :
        _showStarredBtn("Favorite", *this, nullptr, IntRect(30, 70, 120, 30), Icon::DrawFavorite),
        _hidMapperBtn("Mapper", *this, nullptr, IntRect(165, 70, 120, 30), Icon::DrawController),
        _gameGuideBtn("Game Guide", *this, nullptr, IntRect(30, 105, 120, 30), Icon::DrawGuide),
        _searchBtn("Search", *this, nullptr, IntRect(165, 105, 120, 30), Icon::DrawSearch),
        _arBtn("ActionReplay", *this, nullptr, IntRect(30, 140, 120, 30)),
        _toolsBtn("Tools", *this, nullptr, IntRect(165, 140, 120, 30), Icon::DrawTools),

        _AddFavoriteBtn(*this, &PluginMenuHome::_StarItem, IntRect(50, 30, 25, 25), Icon::DrawAddFavorite),
        _InfoBtn(*this, &PluginMenuHome::_DisplayNote, IntRect(90, 30, 25, 25), Icon::DrawInfo, false),

        _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose),
        _keyboardBtn(*this, nullptr, IntRect(130, 30, 25, 25), Icon::DrawKeyboard, false),
        _controllerBtn(*this, nullptr, IntRect(170, 30, 25, 25), Icon::DrawGameController, false)
    {
        _root = _folder = new MenuFolderImpl(name);
        _starredConst = _starred = new MenuFolderImpl("Favorites");

        _starMode = false;
        _selector = 0;
        _selectedTextSize = 0;
        _scrollOffset = 0.f;
        _maxScrollOffset = 0.f;
        _reverseFlow = false;
        _showVersion = false;
        _versionPosX = 0;

        _noteTB = nullptr;

        // Set rounding
        _showStarredBtn.RoundedRatio(7);
        _hidMapperBtn.RoundedRatio(7);
        _gameGuideBtn.RoundedRatio(7);
        _searchBtn.RoundedRatio(7);
        _arBtn.RoundedRatio(7);
        _toolsBtn.RoundedRatio(7);

        // Temporary disable unused buttons
        _hidMapperBtn.IsLocked = true;
        _arBtn.IsLocked = true;

        // Decode strings
        g_ctrpfString = new char[19];
        g_bymeString = new char[13];

        std::memset(g_ctrpfString, 0, 19);
        std::memset(g_bymeString, 0, 13);

        decoder(g_ctrpfString, g_ctrpf, 18);
        decoder(g_bymeString, g_byme, 12);
    }

    bool PluginMenuHome::operator()(EventList& eventList, int& mode, Time& delta)
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

        // If Favorites button is (de) activated
        if (_showStarredBtn())
        {
            static int bak = 0;
            std::swap(bak, _selector);
            _starMode = !_starMode;

            MenuFolderImpl* f = _starMode ? _starred : _folder;
            if (f->ItemsCount() == 0)
            {
                _InfoBtn.Enable(false);
                _AddFavoriteBtn.Enable(false);
                _keyboardBtn.Enable(false);
                _selectedTextSize = 0;
            }
            else
            {
                MenuEntryImpl* e = reinterpret_cast<MenuEntryImpl *>(f->_items[_selector]);
                _InfoBtn.Enable(e->note.size() > 0);
                _keyboardBtn.Enable(e->MenuFunc != nullptr);
                _AddFavoriteBtn.Enable(true);
                _AddFavoriteBtn.SetState(e->_IsStarred());

                // Update entry infos
                _selectedTextSize = Renderer::GetTextSize(e->name.c_str());
                _maxScrollOffset = static_cast<float>(_selectedTextSize) - 200.f;
                _scrollClock.Restart();
                _scrollOffset = 0.f;
                _reverseFlow = false;
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

        if (_controllerBtn())
        {
            MenuFolderImpl* f = _starMode ? _starred : _folder;
            MenuEntryImpl* e = reinterpret_cast<MenuEntryImpl *>(f->_items[_selector]);
            MenuEntry *entry = e->_owner;

            if (entry != nullptr)
            {
                if (entry->Hotkeys.Count() == 1)
                {
                    entry->Hotkeys[0].AskForKeys();
                    if (entry->Hotkeys._callback != nullptr)
                        entry->Hotkeys._callback(entry, 0);
                    entry->RefreshNote();
                }
                else if (entry->Hotkeys.Count())
                {
                    entry->Hotkeys.AskForKeys();
                }
            }
        }

        if (_keyboardBtn())
        {
            MenuFolderImpl* f = _starMode ? _starred : _folder;
            MenuEntryImpl* e = reinterpret_cast<MenuEntryImpl *>(f->_items[_selector]);
            e->MenuFunc(e->_owner);
        }

        return (_closeBtn());
    }

    void PluginMenuHome::Append(MenuItem* item) const
    {
        _folder->Append(item);
    }

    void PluginMenuHome::Refresh(void)
    {
        // If the currently selected folder is root
        // Nothing to do
        if (_folder->_container != nullptr)
        {
            // If current folder is hidden, close it
            while (!_folder->_isVisible)
            {
                MenuFolderImpl *p = _folder->_Close(_selector);

                if (p)
                {
                    _folder = p;
                    if (_selector >= 1)
                        _selector--;
                }
                else
                    break;
            }
        }

        // Starred folder
        do
        {
            // If the currently selected folder is root
            // Nothing to do
            if (_starred->_container == nullptr)
                break;

            // If current folder is hidden, close it
            while (!_starred->_isVisible)
            {
                MenuFolderImpl *p = _starred->_Close(_selector, true);

                if (p)
                {
                    _starred = p;
                    if (_selector >= 1)
                        _selector--;
                }
                else
                    break;
            }
        } while (true);

        // Check for the validity of _selector range
        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        if (_selector >= folder->ItemsCount())
            _selector = 0;

    }

    //###########################################
    // Process Event
    //###########################################
    void PluginMenuHome::_ProcessEvent(Event& event)
    {
        static Clock fastScroll;
        static Clock inputClock;
        static MenuItem* last = nullptr;

        MenuFolderImpl* folder = _starMode ? _starred : _folder;
        MenuItem* item;

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
                        default: break;
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
                        MenuFolderImpl* p = folder->_Close(_selector, _starMode);
                        if (p != nullptr)
                        {
                            if (_starMode)
                            {
                                _starred = p;
                            }
                            else
                            {
                                _folder = p;
                            }
                        }
                        break;
                    }
                    default: break;
                }
                break;
            } // End Key::Pressed event       
            default: break;
        } // End switch

        folder = _starMode ? _starred : _folder;

		if (_selector >= folder->ItemsCount())
			_selector = 0;
        /*
        ** Scrolling text variables
        *********************************/
        if (folder->ItemsCount() > 0 && event.key.code != Key::Touchpad && (event.type < Event::TouchBegan || event.type > Event::TouchSwipped))
        {
            item = folder->_items[_selector];
            _selectedTextSize = Renderer::GetTextSize(item->name.c_str());
            _maxScrollOffset = static_cast<float>(_selectedTextSize) - 200.f;
            _scrollClock.Restart();
            _scrollOffset = 0.f;
            _reverseFlow = false;
        }
        else if (folder->ItemsCount() == 0)
        {
            _selectedTextSize = 0;
			_AddFavoriteBtn.SetState(false);
			_AddFavoriteBtn.Enable(false);
			_InfoBtn.Enable(false);
			_InfoBtn.SetState(false);
			_keyboardBtn.Enable(false);
			_keyboardBtn.SetState(false);
        }

        /*
        ** Update favorite state
        **************************/
        if (folder->ItemsCount() > 0 && _selector < folder->ItemsCount())
        {
            item = folder->_items[_selector];
            _AddFavoriteBtn.SetState(item->_IsStarred());

            if (last != item)
            {
                if (item->_type == MenuType::Entry)
                {
                    MenuEntryImpl* e = reinterpret_cast<MenuEntryImpl *>(item);
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

                if (item->GetNote().size() > 0)
                {
                    static IntRect noteRect(40, 30, 320, 180);
                    _noteTB = new TextBox(item->name, item->GetNote(), noteRect);
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

    void PluginMenuHome::_RenderTop(void)
    {
        static Color blank(255, 255, 255);
        static Color silver(160, 160, 160);

        int posY = 25;
        int posX = 40;


        // Draw background
        Window::TopWindow.Draw();

        MenuFolderImpl* folder = _starMode ? _starred : _folder;

        // Draw Title
        int maxWidth = _showVersion ? _versionPosX - 10 : 360;
        int posYbak = posY;
        int width = Renderer::DrawSysString(folder->name.c_str(), posX, posY, maxWidth, blank);
        Renderer::DrawLine(posX, posY, width, blank);
        posY += 7;

        if (_showVersion && !_starMode && !folder->HasParent())
            Renderer::DrawSysString(_versionStr.c_str(), _versionPosX, posYbak, 360, blank);

        // Draw Entry
        int max = folder->ItemsCount();
        if (max == 0)
            return;
        int i = std::max(0, _selector - 6);
        max = std::min(max, (i + 8));

        for (; i < max; i++)
        {
            MenuItem* item = folder->_items[i];
            if (i == _selector)
            {
                Renderer::MenuSelector(posX - 5, posY - 3, 320, 20);
            }
            if (item->_type == MenuType::Entry)
            {
                MenuEntryImpl* entry = reinterpret_cast<MenuEntryImpl *>(item);
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

    void PluginMenuHome::_RenderBottom(void)
    {
        Color& blank = Color::Blank;
        static Clock creditClock;
        static bool framework = true;

        Renderer::SetTarget(BOTTOM);

        Window::BottomWindow.Draw();

        int posY = 205;

        if (framework)
            Renderer::DrawString(g_ctrpfString, 100, posY, blank);
        else
            Renderer::DrawString(g_bymeString, 124, posY, blank);

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
        _controllerBtn.Draw();

        _closeBtn.Draw();
    }

    //###########################################
    // Update menu
    //###########################################
    void PluginMenuHome::_Update(Time delta)
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

        // Buttons visibility

        // If current folder is empty

        MenuFolderImpl *folder = _starMode ? _starred : _folder;
        
        if (folder->ItemsCount() == 0)
        {
            _AddFavoriteBtn.Enable(false);
            _InfoBtn.Enable(false);
            _keyboardBtn.Enable(false);
            _controllerBtn.Enable(false);
        }

        // If selected object is a folder
        else if ((*folder)[_selector]->IsFolder())
        {
            MenuFolderImpl *e = reinterpret_cast<MenuFolderImpl *>((*folder)[_selector]);

            // A folder will not have a menufunc
            _keyboardBtn.Enable(false);
            // Check if folder has a note
            _InfoBtn.Enable(e->note.size());
            // Enable AddFavorites icon
            _AddFavoriteBtn.Enable(true);
            _AddFavoriteBtn.SetState(e->_IsStarred());
            _controllerBtn.Enable(false);
        }

        // If selected object is an entry
        else if ((*folder)[_selector]->IsEntry())
        {
            MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl *>((*folder)[_selector]);
            std::string &note = e->GetNote();
            // Check if entry has a menu func
            _keyboardBtn.Enable(e->MenuFunc != nullptr);
            // Check if entry has a note
            _InfoBtn.Enable(note.size());
            // Enable AddFavorites icon
            _AddFavoriteBtn.Enable(true);
            _AddFavoriteBtn.SetState(e->_IsStarred());
            // Enable controller icon
            bool enable = false;
            if (e->_owner != nullptr)
                enable = e->_owner->Hotkeys.Count() > 0;
            _controllerBtn.Enable(enable);

            if (e->HasNoteChanged() && _noteTB != nullptr)
            {
                _noteTB->Update(e->name, e->GetNote());
                e->HandledNoteChanges();
            }
        }

        // An error is happening
        else
        {
            _AddFavoriteBtn.Enable(false);
            _InfoBtn.Enable(false);
            _keyboardBtn.Enable(false);
            _controllerBtn.Enable(false);
        }

        // Buttons status

        bool isTouched = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());

        _showStarredBtn.Update(isTouched, touchPos);
        _gameGuideBtn.Update(isTouched, touchPos);
        _arBtn.Update(isTouched, touchPos);
        _toolsBtn.Update(isTouched, touchPos);
        _hidMapperBtn.Update(isTouched, touchPos);
        _searchBtn.Update(isTouched, touchPos);

        _InfoBtn.Update(isTouched, touchPos);
        _AddFavoriteBtn.Update(isTouched, touchPos);
        _keyboardBtn.Update(isTouched, touchPos);
        _controllerBtn.Update(isTouched, touchPos);
        _closeBtn.Update(isTouched, touchPos);
    }

    void PluginMenuHome::_TriggerEntry(void)
    {
        MenuFolderImpl* folder = _starMode ? _starred : _folder;


        if (_selector >= folder->ItemsCount())
            return;

        MenuItem* item = folder->_items[_selector];

        /*
        ** MenuEntryImpl
        **************/
        if (item->_type == MenuType::Entry)
        {
            MenuEntryImpl* entry = reinterpret_cast<MenuEntryImpl *>(item);

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
            MenuFolderImpl* p = reinterpret_cast<MenuFolderImpl *>(item);
            p->_Open(folder, _selector, _starMode);
            if (_starMode)
                _starred = p;
            else
                _folder = p;
            _selector = 0;
        }
    }


    void PluginMenuHome::_DisplayNote(void)
    {
        if (_noteTB != nullptr)
        {
            if (_noteTB->IsOpen())
                _noteTB->Close();
            else
                _noteTB->Open();
        }
    }

    void PluginMenuHome::_StarItem(void)
    {
        MenuFolderImpl* folder = _starMode ? _starred : _folder;

        if (_selector >= folder->ItemsCount())
            return;

        MenuItem* item = folder->_items[_selector];

        if (item)
        {
            bool star = item->_TriggerStar();

            if (star)
                _starredConst->Append(item, true);
            else
            {
                UnStar(item);
            }
        }
    }

    void PluginMenuHome::UnStar(MenuItem* item)
    {
        MenuFolderImpl* folder = _starMode ? _starred : _folder;

        if (item != nullptr)
        {
			item->_isStarred = false;

            int count = _starredConst->ItemsCount();

            if (count == 1)
            {
                _starredConst->_items.clear();
            }
            else
            {
                for (int i = 0; i < count; i++)
                {
                    MenuItem* it = _starredConst->_items[i];

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

    void    PluginMenuHome::Init(void)
    {
        MenuFolderImpl* folder = _starMode ? _starred : _folder;
        MenuItem    *item = folder->ItemsCount() != 0 ? folder->_items[0] : nullptr;

        // Init buttons state
        _AddFavoriteBtn.Enable(folder->ItemsCount() != 0);
        _InfoBtn.Enable(item != nullptr ? !item->GetNote().empty() : false);
    }

    void    PluginMenuHome::TriggerSearch(bool state)
    {
        _searchBtn.IsLocked = !state;
    }

    void    PluginMenuHome::TriggerActionReplay(bool state)
    {
        _arBtn.IsLocked = !state;
    }

    void    PluginMenuHome::AddPluginVersion(u32 version)
    {
        char buffer[100];

        sprintf(buffer, "[%d.%d.%d]", version & 0xFF, (version >> 8) & 0xFF, version >> 16);
        _versionStr.clear();
        _versionStr = buffer;

        float width = Renderer::GetTextSize(buffer);

        _versionPosX = 360 - (width + 1);
        _showVersion = true;
    }
}

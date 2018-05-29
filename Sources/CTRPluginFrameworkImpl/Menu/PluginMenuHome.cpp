#include "CTRPluginFrameworkImpl/Menu/PluginMenuHome.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"
#include "CTRPluginFramework/Menu/MenuFolder.hpp"

#include <cstring>

namespace CTRPluginFramework
{
    static char* g_ctrpfString = nullptr;
    static char* g_bymeString = nullptr;

    static u32 g_size[2] = { 0 };
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
        _showStarredBtn("Favorite", *this,  &PluginMenuHome::_showStarredBtn_OnClick, IntRect(30, 70, 120, 30), Icon::DrawFavorite),
        _hidMapperBtn("Mapper", *this, nullptr, IntRect(165, 70, 120, 30), Icon::DrawController),
        _gameGuideBtn("Game Guide", *this, &PluginMenuHome::_gameGuideBtn_OnClick, IntRect(30, 105, 120, 30), Icon::DrawGuide),
        _searchBtn("Search", *this, &PluginMenuHome::_searchBtn_OnClick, IntRect(165, 105, 120, 30), Icon::DrawSearch),
        _arBtn("ActionReplay", *this, &PluginMenuHome::_actionReplayBtn_OnClick, IntRect(30, 140, 120, 30)),
        _toolsBtn("Tools", *this, &PluginMenuHome::_toolsBtn_OnClick, IntRect(165, 140, 120, 30), Icon::DrawTools),

        _AddFavoriteBtn(*this, &PluginMenuHome::_StarItem, IntRect(50, 30, 25, 25), Icon::DrawAddFavorite),
        _InfoBtn(*this, &PluginMenuHome::_InfoBtn_OnClick, IntRect(90, 30, 25, 25), Icon::DrawInfo, false),

       // _closeBtn(*this, nullptr, IntRect(275, 24, 20, 20), Icon::DrawClose),
        _keyboardBtn(*this, &PluginMenuHome::_keyboardBtn_OnClick, IntRect(130, 30, 25, 25), Icon::DrawKeyboard, false),
        _controllerBtn(*this, &PluginMenuHome::_controllerBtn_OnClick, IntRect(170, 30, 25, 25), Icon::DrawGameController, false),
        _noteTB("", "", IntRect(40, 30, 320, 180))
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

        _mode = 0;

        // Set rounding
        _showStarredBtn.RoundedRatio(7);
        _hidMapperBtn.RoundedRatio(7);
        _gameGuideBtn.RoundedRatio(7);
        _searchBtn.RoundedRatio(7);
        _arBtn.RoundedRatio(7);
        _toolsBtn.RoundedRatio(7);

        // Temporary disable unused buttons
        _hidMapperBtn.IsLocked = true;

        // Decode strings
        g_ctrpfString = new char[19];
        g_bymeString = new char[13];

        std::memset(g_ctrpfString, 0, 19);
        std::memset(g_bymeString, 0, 13);

        decoder(g_ctrpfString, g_ctrpf, 18);
        decoder(g_bymeString, g_byme, 12);
        g_size[0] = Renderer::LinuxFontSize(g_ctrpfString);
        g_size[1] = Renderer::LinuxFontSize(g_bymeString);

        _uiContainer += &_showStarredBtn;
        _uiContainer += &_hidMapperBtn;
        _uiContainer += &_gameGuideBtn;
        _uiContainer += &_searchBtn;
        _uiContainer += &_arBtn;
        _uiContainer += &_toolsBtn;
       // _uiContainer += &_closeBtn;
        _uiContainer += &_keyboardBtn;
        _uiContainer += &_controllerBtn;
        _uiContainer += &_AddFavoriteBtn;
        _uiContainer += &_InfoBtn;

        // Are the buttons locked ?
        _arBtn.IsLocked = !Preferences::Settings.AllowActionReplay;
        _searchBtn.IsLocked = !Preferences::Settings.AllowSearchEngine;
    }

    bool PluginMenuHome::operator()(EventList& eventList, int& mode, Time& delta)
    {
        //static int note = 0;
        static Task top([](void *arg)
        {
            PluginMenuHome *home = reinterpret_cast<PluginMenuHome *>(arg);

             Renderer::SetTarget(TOP);
            if (home->_noteTB.IsOpen())
                home->_noteTB.Draw();
            else
                home->_RenderTop();

            return (s32)0;

        }, this, Task::AppCores);

        _mode = mode;

        // Process events
        if (_noteTB.IsOpen())
        {
            for (int i = 0; i < eventList.size(); i++)
                if (_noteTB.ProcessEvent(eventList[i]) == false)
                {
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
        top.Start();
        /*Renderer::SetTarget(TOP);
        if (_noteTB.IsOpen())
            _noteTB.Draw();
        else
            _RenderTop();*/

        // RenderBottom
        _RenderBottom();

        top.Wait();

        // Execute UIControls
        _uiContainer.ExecuteAll();

        mode = _mode;

        return (Window::BottomWindow.MustClose());
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
            while (!_folder->Flags.isVisible)
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
            while (!_starred->Flags.isVisible)
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

#define IsUnselectableEntry(item) (item->IsEntry() && item->AsMenuEntryImpl()._flags.isUnselectable)
    static u32  SelectableEntryCount(MenuFolderImpl &folder)
    {
        u32 count = 0;

        for (u32 i = 0; i < folder.ItemsCount(); i++)
        {
            MenuItem *item = folder[i];

            if (item->IsEntry() && item->AsMenuEntryImpl()._flags.isUnselectable)
                continue;
            else
                count++;
        }
        return (count);
    }

    static void ScrollUp(int &selector, MenuFolderImpl &folder, int step)
    {
        // If there's no selectable entry in the folder, return
        if (!SelectableEntryCount(folder))
            return;

        // We're already at the begining
        if (selector == 0)
        {
            // Else select last item
            selector = folder.ItemsCount() - 1;
            // If entry is unselectable scroll again
            if (IsUnselectableEntry(folder[selector]))
                ScrollUp(selector, folder, step);
            return;
        }
        // Else go up
        selector -= step;
        if (selector < 0)
            selector = 0;
        // If entry is unselectable, scroll again
        if (IsUnselectableEntry(folder[selector]))
        {
            step = step > 1 ? step - 1 : 1;
            ScrollUp(selector, folder, step);
        }
    }

    static void ScrollDown(int &selector, MenuFolderImpl &folder, int step)
    {
        // If there's no selectable entry in the folder, return
        if (!SelectableEntryCount(folder))
            return;

        // We're already at the end
        if (selector == folder.ItemsCount() - 1)
        {
            // Else select first item
            selector = 0;
            // If entry is unselectable scroll again
            if (IsUnselectableEntry(folder[selector]))
                ScrollDown(selector, folder, step);
            return;
        }
        // Else go down
        selector += step;
        if (selector >= folder.ItemsCount())
            selector = folder.ItemsCount() - 1;
        // If entry is unselectable, scroll again
        if (IsUnselectableEntry(folder[selector]))
        {
            step = step > 1 ? step - 1 : 1;
            ScrollDown(selector, folder, step);
        }
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
                            ScrollUp(_selector, *folder, 1);
                            break;
                        }
                        case Key::CPadDown:
                        case Key::DPadDown:
                        {
                            ScrollDown(_selector, *folder, 1);
                            break;
                        }
                        case Key::CPadLeft:
                        case Key::DPadLeft:
                        {
                            ScrollUp(_selector, *folder, 4);
                            break;
                        }
                        case Key::CPadRight:
                        case Key::DPadRight:
                        {
                            ScrollDown(_selector, *folder, 4);
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
                        ScrollUp(_selector, *folder, 1);
                        fastScroll.Restart();
                        break;
                    }
                    case Key::CPadDown:
                    case Key::DPadDown:
                    {
                        ScrollDown(_selector, *folder, 1);
                        fastScroll.Restart();
                        break;
                    }
                    case Key::CPadLeft:
                    case Key::DPadLeft:
                    {
                        ScrollUp(_selector, *folder, 4);
                        fastScroll.Restart();
                        break;
                    }
                    case Key::CPadRight:
                    case Key::DPadRight:
                    {
                        ScrollDown(_selector, *folder, 4);
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
                        MenuFolderImpl *newFolder = folder->_Close(_selector, _starMode);

                        // Call the MenuEntry::OnAction callback if there's one
                        if (folder->_owner != nullptr && folder->_owner->OnAction != nullptr)
                            folder->_owner->OnAction(*_folder->_owner, ActionType::Closing);

                        // Switch current folder
                        if (newFolder != nullptr)
                        {
                            if (_starMode)
                                _starred = newFolder;
                            else
                                _folder = newFolder;
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

                if (item->GetNote().size() > 0)
                {
                    _noteTB.Update(item->name, item->GetNote());
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
        const Color &selected = Preferences::Settings.MenuSelectedItemColor;
        const Color &unselected = Preferences::Settings.MenuUnselectedItemColor;
        const Color &maintext = Preferences::Settings.MainTextColor;

        int posY = 25;
        int posX = 40;


        // Draw background
        Window::TopWindow.Draw();

        MenuFolderImpl* folder = _starMode ? _starred : _folder;

        // Draw Title
        int maxWidth = _showVersion ? _versionPosX - 10 : 360;
        int posYbak = posY;
        int width = Renderer::DrawSysString(folder->name.c_str(), posX, posY, maxWidth, maintext);
        Renderer::DrawLine(posX, posY, width, maintext);
        posY += 7;

        if (_showVersion && !_starMode && !folder->HasParent())
            Renderer::DrawSysString(_versionStr.c_str(), _versionPosX, posYbak, 360, maintext);

        // Draw Entry
        u32  drawSelector = SelectableEntryCount(*folder);
        int max = folder->ItemsCount();
        if (max == 0)
            return;
        int i = std::max(0, _selector - 6);
        max = std::min(max, (i + 8));

        for (; i < max; i++)
        {
            MenuItem    *item = folder->_items[i];
            ItemFlags   flags = item->Flags;
            const char  *name = item->name.c_str();
            const Color  &fg = i == _selector ? selected : unselected;
            float       offset = i == _selector ? _scrollOffset : 0.f;

            // Draw separator if needed
            if (flags.useSeparatorBefore)
            {
                if (flags.useStippledLineForBefore)
                    Renderer::DrawStippledLine(posX, posY - 1, 320, unselected, 1);
                else
                    Renderer::DrawLine(posX, posY - 1, 320, unselected, 1);
            }

            // Draw cursor
            if (drawSelector && i == _selector)
                Renderer::MenuSelector(posX - 5, posY - 3, 330, 20);

            // Draw entry
            if (item->_type == MenuType::Entry)
            {
                MenuEntryImpl   *entry = reinterpret_cast<MenuEntryImpl *>(item);

                if (entry->GameFunc != nullptr)
                    Renderer::DrawSysCheckBox(name, posX, posY, 350, fg, entry->IsActivated(), offset);
                else
                {
                    if (entry->MenuFunc != nullptr && !entry->_flags.isUnselectable)
                        Icon::DrawSettings(posX, posY);

                    Renderer::DrawSysString(name, posX + 20, posY, 350, fg, offset);
                    posY += 1;
                }
            }
            // Draw folder
            else
            {
                Renderer::DrawSysFolder(name, posX, posY, 350, fg, offset);
            }

            // Draw separator if needed
            if (flags.useSeparatorAfter)
            {
                if (flags.useStippledLineForAfter)
                    Renderer::DrawStippledLine(posX, posY - 1, 320, unselected, 1);
                else
                    Renderer::DrawLine(posX, posY - 1, 320, unselected, 1);
            }
            posY += 4;
        }
    }

    //###########################################
    // Render Bottom Screen
    //###########################################

    void PluginMenuHome::_RenderBottom(void)
    {
        const Color& blank = Color::Blank;
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

        // Draw UIControls
        _uiContainer.Draw();
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

        MenuFolderImpl *folder = _starMode ? _starred : _folder;

        // If current folder is empty
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

            if (e->HasNoteChanged())
            {
                _noteTB.Update(e->name, e->GetNote());
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

        // Update UIControls
        _uiContainer.Update(isTouched, touchPos);

        Window::BottomWindow.Update(isTouched, touchPos);
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

            if (entry->_flags.isUnselectable)
                return;
            // If the entry has a valid funcpointer
            if (entry->GameFunc != nullptr)
            {
                // Change the state
                bool just = entry->_flags.justChanged;
                bool state = entry->_TriggerState();

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
            else if (entry->MenuFunc != nullptr)
            {
                entry->MenuFunc(entry->_owner);
            }
        }
        /*
        ** MenuFolderImpl
        ****************/
        else
        {
            MenuFolderImpl* p = reinterpret_cast<MenuFolderImpl *>(item);

            // If a MenuFolder exists and has a callback
            if (p->_owner != nullptr && p->_owner->OnAction != nullptr)
            {
                // If the callabck tells us to not open the folder
                if (!(p->_owner->OnAction(*p->_owner, ActionType::Opening)))
                    return;
            }
            p->_Open(folder, _selector, _starMode);
            if (_starMode)
                _starred = p;
            else
                _folder = p;
            _selector = 0;
        }
    }

    void    PluginMenuHome::_showStarredBtn_OnClick(void)
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

    void    PluginMenuHome::_controllerBtn_OnClick(void)
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

    void    PluginMenuHome::_keyboardBtn_OnClick(void)
    {
        MenuFolderImpl *f = _starMode ? _starred : _folder;
        MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl *>((*f)[_selector]);

        if (e->MenuFunc != nullptr)
            e->MenuFunc(e->_owner);
    }

    void PluginMenuHome::_actionReplayBtn_OnClick()
    {
        _mode = 4;
    }

    void    PluginMenuHome::_gameGuideBtn_OnClick(void)
    {
        _mode = 2;
    }

    void    PluginMenuHome::_searchBtn_OnClick(void)
    {
        _mode = 3;
    }

    void    PluginMenuHome::_toolsBtn_OnClick(void)
    {
        _mode = 5;
    }

    void    PluginMenuHome::_InfoBtn_OnClick(void)
    {
        if (_noteTB.IsOpen())
            _noteTB.Close();
        else
            _noteTB.Open();
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
			item->Flags.isStarred = false;

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

    void    PluginMenuHome::Close(MenuFolderImpl *folder)
    {
        if (folder != _root)
        {
            if(_folder == folder)
                _folder = _folder->_Close(_selector, false);
            if (_starred == folder)
                _starred = _starred->_Close(_selector, true);
        }
    }
}

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
#include "NTRImpl.hpp"

namespace CTRPluginFramework
{
    PluginMenuImpl  *PluginMenuImpl::_runningInstance = nullptr;

    PluginMenuImpl::PluginMenuImpl(std::string &name, std::string &about) : 
    _hexEditor(0x00100000),
    _freeCheats(_hexEditor),
    _home(new PluginMenuHome(name)),
    _search(new PluginMenuSearch(_hexEditor, _freeCheats)),
    _tools(new PluginMenuTools(about, _hexEditor, _freeCheats)),
    _executeLoop(new PluginMenuExecuteLoop()),
    _guide(new GuideReader())
    {
        _isOpen = false;
        _wasOpened = false;
        _pluginRun = true;
        _showMsg = true;
    }

    PluginMenuImpl::~PluginMenuImpl(void)
    {
        ProcessImpl::Play(false);
        delete _home;
        delete _search;
        delete _tools;
        delete _executeLoop;
        delete _guide;
    }  

    void    PluginMenuImpl::Append(MenuItem *item) const
    {
        _home->Append(item);
    }

    void    PluginMenuImpl::Callback(CallbackPointer callback)
    {
        _callbacks.push_back(callback);
    }

    void    PluginMenuImpl::RemoveCallback(CallbackPointer callback)
    {
        for (int i = _callbacks.size() - 1; i >= 0; i--)
        {
            if (_callbacks[i] == callback)
            {
                _callbacks.erase(_callbacks.begin() + i);
            }
        }
    }

    using KeyVector = std::vector<Key>;

    class   KeySequenceImpl
    {
    public:

        KeySequenceImpl(KeyVector sequence) :
            _sequence(sequence), _indexInSequence(0)
        {
        }

        ~KeySequenceImpl() {}

        /**
        * \brief Check the sequence
        * \return true if the sequence is completed, false otherwise
        */
        bool  operator()(void)
        {
            if (Controller::IsKeyDown(_sequence[_indexInSequence]))
            {
                _indexInSequence++;

                if (_indexInSequence >= _sequence.size())
                {
                    _indexInSequence = 0;
                    return (true);
                }

                _timer.Restart();
            }

            if (_timer.HasTimePassed(Seconds(1.f)))
            {
                _indexInSequence = 0;
                _timer.Restart();
            }

            return (false);
        }

    private:
        KeyVector   _sequence;
        Clock       _timer;
        int         _indexInSequence;

    };

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
        std::vector<Event>     eventList;

        // Set _runningInstance to this menu
        _runningInstance = this;

        // Load settings
        Preferences::LoadSettings();

        // Refresh hid
        Controller::Update();

        // If Start is pressed, don't auto enable the cheats
        if (Controller::IsKeyPressed(Key::Start) || Controller::IsKeyDown(Key::Start))
            Preferences::AutoLoadCheats = false;

        _tools->UpdateSettings();
        
        // Load favorites
        if (Preferences::AutoLoadFavorites)
            Preferences::LoadSavedFavorites();

         // Enable cheats
        if (Preferences::AutoLoadCheats)
            Preferences::LoadSavedEnabledCheats();

        // Load FreeCheats
        Preferences::LoadFreeCheats();

        // Load custom hotkeys
        Preferences::LoadHotkeysFromFile();

        // Update PluginMenuHome variables
        home.Init();

        if (_showMsg)
            OSD::Notify("Plugin ready !", Color(255, 255, 255), Color());

        // Main loop
        while (_pluginRun)
        {
            // Check Event
            eventList.clear();
            while (manager.PollEvent(event))
            {
                // If it's a KeyPressed event
                if (event.type == Event::KeyPressed && inputClock.HasTimePassed(Milliseconds(500)))
                {
                    bool isHotkeysDown = false;

                    // Check that MenuHotkeys are pressed
                    for (int i = 0; i < 16; i++)
                    {
                        u32 key = Preferences::MenuHotkeys & (1u << i);
                        
                        if (key && event.key.code == key)
                        {
                            if (Controller::IsKeysDown(Preferences::MenuHotkeys ^ key))
                                isHotkeysDown = true;
                        }
                    }

                    // If MenuHotkeys are pressed but not Luma's default hotkey
                    if (isHotkeysDown && !(Preferences::MenuHotkeys == Key::Select && Controller::IsKeysDown(Key::L + Key::DPadDown)))
                    {
                        if (_isOpen)
                        {
                            ProcessImpl::Play(true);
                            _isOpen = false;
                            if (Preferences::InjectBOnMenuClose)
                                Controller::InjectKey(Key::B);

                            // Save settings
                            Preferences::WriteSettings();
                        }
                        else
                        {
                            ProcessImpl::Pause(true);
                            _isOpen = true;
                            _wasOpened = true;
                        }
                        inputClock.Restart();
                    }                       
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

                    // Save settings
                    Preferences::WriteSettings();
                }
            }
            else
            {
                // Execute activated cheats
                executer();

                // Execute callbacks
                for (int i = 0; i < _callbacks.size(); i++)
                {
                    _callbacks[i]();
                }

                // Check filenames
                if (Preferences::DisplayFilesLoading)
                {
                    extern char                **g_filenames;
                    extern int                 g_index;
                    extern LightLock           g_OpenFileLock;

                    if (g_index)
                    {
                        if (!OSD::TryLock())
                        {
                            if (!LightLock_TryLock(&g_OpenFileLock))
                            {
                                while (g_index)
                                {
                                    OSD::Notify(g_filenames[g_index - 1]);
                                    g_index--;
                                }

                                LightLock_Unlock(&g_OpenFileLock);
                            }

                            OSD::Unlock();
                        }
                    }
                }
                
                static KeySequenceImpl konamicode({ DPadUp, DPadUp, DPadDown, DPadDown, DPadLeft, DPadRight, DPadLeft, DPadRight, B, A, B, A });

                if (konamicode())
                    NTRImpl::MessColors = !NTRImpl::MessColors;

                OSDImpl::Update();

                if (_wasOpened)
                    _wasOpened = false;
            }
        }

        // Remove Running Instance
        _runningInstance = nullptr;
        return (0);
    }

    void PluginMenuImpl::LoadEnabledCheatsFromFile(const Preferences::Header &header, File &settings)
    {
        if (_runningInstance == nullptr)
            return;

        std::vector<u32>    uids;
        MenuFolderImpl      *folder = _runningInstance->_home->_folder;

        uids.resize(header.enabledCheatsCount);

        settings.Seek(header.enabledCheatsOffset, File::SET);

        if (settings.Read(uids.data(), sizeof(u32) * header.enabledCheatsCount) == 0)
        {
            for (u32 &uid : uids)
            {
                MenuItem *item = folder->GetItem(uid);

                if (item != nullptr && item->IsEntry())
                    reinterpret_cast<MenuEntryImpl *>(item)->Enable();
            }
        }
    }

    void PluginMenuImpl::LoadFavoritesFromFile(const Preferences::Header &header, File &settings)
    {
        if (_runningInstance == nullptr)
            return;

        std::vector<u32>    uids;
        MenuFolderImpl      *folder = _runningInstance->_home->_folder;
        MenuFolderImpl      *starred = _runningInstance->_home->_starredConst;

        uids.resize(header.favoritesCount);

        settings.Seek(header.favoritesOffset, File::SET);

        if (settings.Read(uids.data(), sizeof(u32) * header.favoritesCount) == 0)
        {
            for (u32 &uid : uids)
            {
                MenuItem *item = folder->GetItem(uid);

                if (item != nullptr && !item->_IsStarred())
                {
                    item->_TriggerStar();                   
                    starred->Append(item, true);                    
                }
            }
        }
    }

    void    PluginMenuImpl::LoadHotkeysFromFile(const Preferences::Header &header, File &settings)
    {
        if (_runningInstance == nullptr || header.hotkeysCount == 0)
            return;

        settings.Seek(header.hotkeysOffset, File::SET);

        u32     buffer[50];
        MenuFolderImpl      *folder = _runningInstance->_home->_root;

        for (int count = 0; count < header.hotkeysCount; count++)
        {
            if (settings.Read(buffer, sizeof(u32) * 2) == 0)
            {
                if (settings.Read(&buffer[2], sizeof(u32) * buffer[1]) == 0)
                {
                    MenuItem  *item = folder->GetItem(buffer[0]);

                    if (item == nullptr || !item->IsEntry()) return; ///< An error occurred, abort operation

                    MenuEntry *entry = item->AsMenuEntryImpl().AsMenuEntry();
                    HotkeyManager::OnHotkeyChangeClbk callback = entry->Hotkeys._callback;

                    if (entry->Hotkeys.Count() == buffer[1])
                    {
                        for (int i = 0; i < buffer[1]; i++)
                        {
                            entry->Hotkeys[i] = buffer[2 + i];
                            if (callback != nullptr)
                                callback(entry, i);
                        }

                        entry->RefreshNote();
                    }
                    else
                        return; ///< An error occurred so abort operation
                }
            }
        }
    }

    void    PluginMenuImpl::WriteEnabledCheatsToFile(Preferences::Header &header, File &settings)
    {
        if (_runningInstance == nullptr)
            return;

        std::vector<u32>    uids;
        MenuFolderImpl      *folder = _runningInstance->_home->_folder;

        for (MenuItem *item : folder->_items)
        {
            if (item->IsEntry() && reinterpret_cast<MenuEntryImpl *>(item)->IsActivated())
                uids.push_back(item->Uid);
        }

        if (uids.size())
        {                
            u64 offset = settings.Tell();

            if (settings.Write(uids.data(), sizeof(u32) * uids.size()) == 0)
            {
                header.enabledCheatsCount = uids.size();
                header.enabledCheatsOffset = offset;
            }
        }
    }

    void    PluginMenuImpl::WriteFavoritesToFile(Preferences::Header &header, File &settings)
    {
        if (_runningInstance == nullptr)
            return;

        std::vector<u32>    uids;
        MenuFolderImpl      *folder = _runningInstance->_home->_starred;

        for (MenuItem *item : folder->_items)
        {
            uids.push_back(item->Uid);
        }

        if (uids.size())
        {
            u64 offset = settings.Tell();

            if (settings.Write(uids.data(), sizeof(u32) * uids.size()) == 0)
            {
                header.favoritesCount = uids.size();
                header.favoritesOffset = offset;
            }
        }
    }

    void    PluginMenuImpl::ExtractHotkeys(HotkeysVector &hotkeys, MenuFolderImpl *folder, u32 &size)
    {
        if (folder == nullptr)
            return;

        for (MenuItem *item : folder->_items)
        {
            if (item == nullptr)
                continue;

            if (item->IsFolder())
            {
                ExtractHotkeys(hotkeys, reinterpret_cast<MenuFolderImpl*>(item), size);
                continue;
            }                

            MenuEntry *entry = item->AsMenuEntryImpl().AsMenuEntry();

            if (entry && entry->Hotkeys.Count())
            {
                Preferences::HotkeysInfos hInfos = { 0 };

                hInfos.uid = item->Uid;
                hInfos.count = entry->Hotkeys.Count();

                for (int j = 0; j < hInfos.count; j++)
                {
                    Hotkey &hk = entry->Hotkeys[j];

                    hInfos.hotkeys.push_back(hk._keys);
                }

                hotkeys.push_back(hInfos);
                size += 2 + hInfos.count;
            }
        }
    }

    void    PluginMenuImpl::WriteHotkeysToFile(Preferences::Header &header, File &settings)
    {
        if (_runningInstance != nullptr)
        {
            MenuFolderImpl  *root = _runningInstance->_home->_root;
            HotkeysVector   hotkeys;
            u32             size = 0;
            u32             *buffer;

            ExtractHotkeys(hotkeys, root, size);
            
            if (size)
            {
                buffer = new u32[size];
                u64     offset = settings.Tell();
                int     i = 0;

                for (Preferences::HotkeysInfos &hkInfos : hotkeys)
                {
                    buffer[i++] = hkInfos.uid;
                    buffer[i++] = hkInfos.count;
                    for (u32 keys : hkInfos.hotkeys)
                        buffer[i++] = keys;
                }

                if (settings.Write(buffer, sizeof(u32) * size) == 0)
                {
                    header.hotkeysCount = hotkeys.size();
                    header.hotkeysOffset = offset;
                }

                delete [] buffer;
            }
        }
    }

    void    PluginMenuImpl::GetRegionsList(std::vector<Region>& list)
    {
        if (_runningInstance != nullptr)
            _runningInstance->_search->GetRegionsList(list);
    }

    void    PluginMenuImpl::ForceExit(void)
    {
        if (_runningInstance != nullptr)
            _runningInstance->_pluginRun = false;
    }

    void    PluginMenuImpl::UnStar(MenuItem* item)
    {
        if (_runningInstance != nullptr)
        {
            _runningInstance->_home->UnStar(item);
        }
    }

    void    PluginMenuImpl::Refresh(void)
    {
        if (_runningInstance != nullptr)
        {
            _runningInstance->_home->Refresh();
        }
    }

    void    PluginMenuImpl::TriggerSearch(bool state) const
    {
        _home->TriggerSearch(state);
    }

    void    PluginMenuImpl::TriggerActionReplay(bool state) const
    {
        _home->TriggerActionReplay(state);
    }

    void    PluginMenuImpl::TriggerFreeCheats(bool isEnabled) const
    {
        _tools->TriggerFreeCheatsEntry(isEnabled);
    }

    void    PluginMenuImpl::SetHexEditorState(bool isEnabled) const
    {
        _tools->TriggerHexEditor(isEnabled);
    }

    void    PluginMenuImpl::ShowWelcomeMessage(bool showMsg)
    {
        _showMsg = showMsg;
    }

    MenuFolderImpl* PluginMenuImpl::GetRoot() const
    {
        return (_home->_root);
    }

    bool    PluginMenuImpl::IsOpen(void) const
    {
        return (_isOpen);
    }

    bool    PluginMenuImpl::WasOpened(void) const
    {
        return (_wasOpened);
    }

    void PluginMenuImpl::AddPluginVersion(u32 version) const
    {
        _home->AddPluginVersion(version);
    }
}

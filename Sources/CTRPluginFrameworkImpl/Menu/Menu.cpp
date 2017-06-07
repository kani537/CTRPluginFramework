#include "CTRPluginFrameworkImpl/Menu.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFramework/System/Controller.hpp"
#include "CTRPluginFramework/System/Clock.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

#include <algorithm>
#include "CTRPluginFrameworkImpl/Menu/MenuEntryTools.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryFreeCheat.hpp"

namespace CTRPluginFramework
{
    Menu::Menu(std::string title, IconCallback iconCallback)
    {
       // _background = IntRect(30, 20, 340, 200);
        //_border = IntRect(32, 22, 336, 196);
        _selector = 0;
        _folder = new MenuFolderImpl(title);
        _iconCallback = iconCallback;
    }

    Menu::Menu(MenuFolderImpl *folder, IconCallback iconCallback)
    {
       // _background = IntRect(30, 20, 340, 200);
       // _border = IntRect(32, 22, 336, 196);
        _selector = 0;
        _folder = folder;
        _iconCallback = iconCallback;
        if (_folder == nullptr)
            _folder = new MenuFolderImpl("Menu");
    }

    void    Menu::Open(MenuFolderImpl* folder)
    {
        _selector = 0;
        _folder = folder;
    }

    Menu::~Menu(void)
    {
        // TODO: Free every folder's object
    }

    void    Menu::Append(MenuItem *item) const
    {
        _folder->Append(item);
    }

    void    Menu::Remove(MenuItem *item) const
    {
        for (auto iter = _folder->_items.begin(); iter < _folder->_items.end(); ++iter)
        {
            if (*iter == item)
            {
                _folder->_items.erase(iter);
                break;
            }
        }
    }

#define XMAX 360

    void    Menu::Draw(void) const
    {
        Color &blank = Color::Blank;
        Color &silver = Color::DarkGrey;

        int   posY = 25;
        int   posX = 40;

        Window::TopWindow.Draw();

        // Draw title
        int width = Renderer::DrawSysString(_folder->name.c_str(), posX, posY, XMAX, blank);
        Renderer::DrawLine(posX, posY, width, blank);   
        posY += 7;

        // Draw entries
        int max = _folder->ItemsCount();
        if (max == 0) return;

        int i = std::max(0, (int)(_selector - 6));

        max = std::min(max, i + 8);

        if (_iconCallback != nullptr)
        {
            for (; i < max; i++)
            {
                Color &c = i == _selector ? blank : silver;
                MenuItem *item = _folder->_items[i];

                if (i == _selector)
                {
                    Renderer::MenuSelector(posX - 5, posY - 3, 320, 20);
                }

                // Draw Entry / EntryTools / FreeCheat
                if (item->_type != MenuType::Folder)
                {
                    _iconCallback(posX, posY);
                    Renderer::DrawSysString(item->name.c_str(), posX + 20, posY, XMAX, c);
                }
                // Draw Folder
                else
                    Renderer::DrawSysFolder(item->name.c_str(), posX, posY, XMAX, c);

                posY += 4;
            }            
        }
        else
        {
            for (; i < max; i++)
            {
                Color &c = i == _selector ? blank : silver;
                MenuItem *item = _folder->_items[i];

                if (i == _selector)
                {
                    Renderer::MenuSelector(posX - 5, posY - 3, 320, 20);
                }

                // MenuEntryImpl
                if (item->_type == MenuType::Entry)
                {
                    Renderer::DrawSysString(item->name.c_str(), posX + 20, posY, XMAX, c);
                }
                // MenuEntryTools
                else if (item->_type == MenuType::EntryTools)
                {
                    MenuEntryTools *e = reinterpret_cast<MenuEntryTools *>(item);

                    if (e->UseCheckBox)
                    {
                        Icon::DrawCheckBox(posX, posY, e->IsActivated());
                        Renderer::DrawSysString(item->name.c_str(), posX + 20, posY, XMAX, c);
                    }
                    else
                    {
                        if (e->Icon != nullptr)
                            e->Icon(posX, posY);
                        Renderer::DrawSysString(item->name.c_str(), posX + 20, posY, XMAX, c);
                    }
                }
                // MenuEntryFreeCheat
                else if (item->_type == MenuType::FreeCheat)
                {
                    MenuEntryImpl *e = reinterpret_cast<MenuEntryImpl *>(item);

                    Icon::DrawCheckBox(posX, posY, e->IsActivated());
                    Renderer::DrawSysString(item->name.c_str(), posX + 20, posY, XMAX, c);
                }
                // MenuFolderImpl
                else
                    Renderer::DrawSysFolder(item->name.c_str(), posX, posY, XMAX, c);

                posY += 4;
            }      
        }


    }

    int     Menu::ProcessEvent(Event &event, std::string &userchoice)
    {
        if (_folder->ItemsCount() == 0)
        {
            return (-1);
        }

        // Scrolling Event
        if (event.type == Event::KeyDown)
        {
            switch (event.key.code)
            {
                case CPadUp:
                case DPadUp:
                {
                    if (!_input.HasTimePassed(Milliseconds(200)))
                        return (MenuEvent::SelectorChanged);
                    if (_selector > 0)
                        _selector--;
                    else
                        _selector = std::max(0, (int)_folder->ItemsCount() - 1);
                    _input.Restart();
                    break;
                }
                case CPadDown:
                case DPadDown:
                {
                    if (!_input.HasTimePassed(Milliseconds(200)))
                        break;
                    if (_selector < _folder->ItemsCount() - 1)
                        _selector++;
                    else
                        _selector = 0;
                    _input.Restart();
                    break;
                }
                default: break;
            }            
        }
        // Other event
        else if (event.type == Event::KeyPressed)
        {
            MenuItem *item = _folder->_items[_selector];
            switch (event.key.code)
            {
                case A:
                {
                    // MenuEntryImpl
                    if (item->_type == MenuType::Entry)
                    {
                        userchoice = item->name;
                        return (_selector);
                    }
                    // if it's a folder
                    MenuFolderImpl *folder = reinterpret_cast<MenuFolderImpl *>(item);
                    folder->_Open(_folder, _selector);
                    _selector = 0;
                    break;
                }
                case B:
                {
                    MenuFolderImpl *p = _folder->_Close(_selector);
                    if (p != nullptr)
                        _folder = p;
                    else
                        return (-2);
                    break;
                }
                default: break;
            }
        }
        return (-1);
    }

    // Return a MenuEvent value
    int     Menu::ProcessEvent(Event &event, MenuItem **userchoice)
    {
        // If the current folder is empty
        if (_folder->ItemsCount() == 0)
        {
            if (event.type == Event::KeyPressed && event.key.code == Key::B)
            {
                MenuFolderImpl *p = _folder->_Close(_selector);
                if (p != nullptr)
                {
                    _folder = p;
                    if (userchoice)
                        *userchoice = reinterpret_cast<MenuItem *>(p);
                    return (MenuEvent::FolderChanged);
                }
                else
                    return (MenuEvent::MenuClose);
            }
            return (MenuEvent::Error);
        }

        // Scrolling Event
        if (event.type == Event::KeyDown)
        {
            switch (event.key.code)
            {
                case CPadUp:
                case DPadUp:
                {
                    if (!_input.HasTimePassed(Milliseconds(200)))
                        break;
                    if (_selector > 0)
                        _selector--;
                    else
                        _selector = std::max(0, static_cast<int>(_folder->ItemsCount() - 1));
                    _input.Restart();
                    return (MenuEvent::SelectorChanged);
                }
                case CPadDown:
                case DPadDown:
                {
                    if (!_input.HasTimePassed(Milliseconds(200)))
                        break;
                    if (_selector < _folder->ItemsCount() - 1)
                        _selector++;
                    else
                        _selector = 0;
                    _input.Restart();
                    return (MenuEvent::SelectorChanged);
                }
                default: break;
            }            
        }
        // Other event
        else if (event.type == Event::KeyPressed)
        {
            MenuItem *item = _folder->_items[_selector];

            if (userchoice)
                *userchoice = item;
            switch (event.key.code)
            {
                case A:
                {
                    // MenuEntryImpl
                    if (item->_type == MenuType::Entry)
                    {
                        return (MenuEvent::EntrySelected);
                    }
                    // MenuEntryTools
                    else if (item->_type == MenuType::EntryTools)
                    {
                        MenuEntryTools *e = reinterpret_cast<MenuEntryTools *>(item);

                        if (e->UseCheckBox)
                        {
                            e->TriggerState();
                            if (e->Func != nullptr && e->IsActivated())
                                e->Func();
                        }
                            
                        else if (e->Func != nullptr)
                            e->Func();
                        
                        return (MenuEvent::EntrySelected);
                    }
                    // MenuEntryFreeCheat
                    else if (item->_type == MenuType::FreeCheat)
                    {
                        MenuEntryFreeCheat *e = reinterpret_cast<MenuEntryFreeCheat *>(item);

                        bool state = e->TriggerState();
                        
                        if (state)
                            PluginMenuExecuteLoop::Add(e);
                        else
                            PluginMenuExecuteLoop::Remove(e);

                        return (MenuEvent::EntrySelected);
                    }

                    // if it's a folder
                    MenuFolderImpl *folder = reinterpret_cast<MenuFolderImpl *>(item);
                    folder->_Open(_folder, _selector);
                    _folder = folder;
                    _selector = 0;
                    return (MenuEvent::FolderChanged);
                }
                case B:
                {
                    MenuFolderImpl *p = _folder->_Close(_selector);
                    if (p != nullptr)
                    {
                        _folder = p;
                        if (userchoice)
                            *userchoice = reinterpret_cast<MenuItem *>(p);
                        return (MenuEvent::FolderChanged);
                    }
                    else
                        return (MenuEvent::MenuClose);
                    
                }
                default: break;
            }
        }
        return (MenuEvent::Nothing);
    }
}

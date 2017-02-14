#include "CTRPluginFrameworkImpl/Menu.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Renderer.hpp"
#include "CTRPluginFramework/System/Controller.hpp"
#include "CTRPluginFramework/System/Clock.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

#include <algorithm>

namespace CTRPluginFramework
{
    Menu::Menu(std::string title)
    {
        _background = IntRect(30, 20, 340, 200);
        _border = IntRect(32, 22, 336, 196);
        _selector = 0;
        _folder = new MenuFolderImpl(title);
    }

    Menu::Menu(MenuFolderImpl *folder)
    {
        _background = IntRect(30, 20, 340, 200);
        _border = IntRect(32, 22, 336, 196);
        _selector = 0;
        _folder = folder;
        if (_folder == nullptr)
            _folder = new MenuFolderImpl("Menu");
    }

    Menu::~Menu(void)
    {
        // TODO: Free every folder's object
    }

    void    Menu::Append(MenuItem *item)
    {
        _folder->Append(item);
    }

    #define XMAX 360

    void    Menu::Draw(void)
    {
        static Color black = Color();
        static Color blank(255, 255, 255);
        static Color greyblack(15, 15, 15);
        static Color silver(160, 160, 160);
        static Color limegreen(50, 205, 50);

        int   posY = 25;
        int   posX = 40;

        // Draw background
        if (Preferences::topBackgroundImage->IsLoaded() 
            && (Preferences::topBackgroundImage->GetDimensions() <= _background.size))
            Preferences::topBackgroundImage->Draw(_background.leftTop);
        else
        {
            Renderer::DrawRect2(_background, black, greyblack);
            Renderer::DrawRect(_border, blank, false);
        }

        // Draw title
        int width = Renderer::DrawSysString(_folder->name.c_str(), posX, posY, XMAX, limegreen);
        Renderer::DrawLine(posX, posY, width, limegreen);   
        posY += 7;

        // Draw entries
        int max = _folder->ItemsCount();
        if (max == 0) return;

        int i = std::max(0, (int)(_selector - 6));

        max = std::min(max, i + 8);

        for (; i < max; i++)
        {
            Color &c = i == _selector ? blank : silver;
            MenuItem *item = _folder->_items[i];

            if (i == _selector)
            {
                Renderer::MenuSelector(posX - 5, posY - 3, 320, 20);
            }

            if (item->_type == MenuType::Entry)
                Renderer::DrawSysString(item->name.c_str(), posX + 20, posY, XMAX, c);
            else
                Renderer::DrawSysFolder(item->name.c_str(), posX, posY, XMAX, c);

            posY += 4;
        }
    }

    int     Menu::ProcessEvent(Event &event, std::string &userchoice)
    {
        if (_folder->ItemsCount() == 0)
            return (-1);

        // Scrolling Event
        if (event.type == Event::KeyDown)
        {
            switch (event.key.code)
            {
                case CPadUp:
                case DPadUp:
                {
                    if (!_input.HasTimePassed(Milliseconds(400)))
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
                    if (!_input.HasTimePassed(Milliseconds(400)))
                        break;
                    if (_selector < _folder->ItemsCount() - 1)
                        _selector++;
                    else
                        _selector = 0;
                    _input.Restart();
                    break;
                }
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
                    // if it's not a folder
                    if (item->_type == MenuType::Entry)
                    {
                        userchoice = item->name;
                        return (_selector);
                    }

                    // if it's a folder
                    MenuFolderImpl *folder = reinterpret_cast<MenuFolderImpl *>(item);
                    folder->_Open(_folder, _selector);
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
            }
        }
        return (-1);
    }

    int     Menu::ProcessEvent(Event &event, MenuItem **userchoice)
    {
        if (_folder->ItemsCount() == 0)
            return (MenuEvent::Error);

        // Scrolling Event
        if (event.type == Event::KeyDown)
        {
            switch (event.key.code)
            {
                case CPadUp:
                case DPadUp:
                {
                    if (!_input.HasTimePassed(Milliseconds(400)))
                        break;
                    if (_selector > 0)
                        _selector--;
                    else
                        _selector = std::max(0, (int)_folder->ItemsCount() - 1);
                    _input.Restart();
                    return (MenuEvent::SelectorChanged);
                }
                case CPadDown:
                case DPadDown:
                {
                    if (!_input.HasTimePassed(Milliseconds(400)))
                        break;
                    if (_selector < _folder->ItemsCount() - 1)
                        _selector++;
                    else
                        _selector = 0;
                    _input.Restart();
                    return (MenuEvent::SelectorChanged);
                }
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
                    // if it's not a folder
                    if (item->_type == MenuType::Entry)
                    {
                        return (MenuEvent::EntrySelected);
                    }

                    // if it's a folder
                    MenuFolderImpl *folder = reinterpret_cast<MenuFolderImpl *>(item);
                    folder->_Open(_folder, _selector);
                    _folder = folder;
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
            }
        }
        return (MenuEvent::Nothing);
    }
}
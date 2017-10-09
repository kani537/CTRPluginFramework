#ifndef CTRPLUGINFRAMEWORKIMPL_MENUIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_MENUIMPL_HPP

#include "types.h"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuFolderImpl.hpp"
#include "CTRPluginFrameworkImpl/System/Events.hpp"
#include "CTRPluginFramework/System/Time.hpp"

namespace CTRPluginFramework
{
    enum MenuEvent
    {
        Error = -1,
        EntrySelected = -2,
        FolderChanged = -3,
        MenuClose = -4,
        SelectorChanged = -5,
        Nothing = -6
    };

    class Menu
    {
        using IconCallback = int (*)(int, int);
    public:

        Menu(const std::string &title, const std::string &footer = "", IconCallback iconCallback = nullptr);
        Menu(MenuFolderImpl *folder, IconCallback iconCallback = nullptr);
        ~Menu(void);

        void            Append(MenuItem *item) const;
        void            Remove(MenuItem *item);
        MenuFolderImpl  *GetFolder(void) const;
        MenuItem        *GetSelectedItem(void) const;

        void    Draw(void) const;
        MenuFolderImpl    *Open(MenuFolderImpl *folder, int selector = 0);
        //void    Update(Time delta);

        /*
        ** Return value: 
        ** -1 : error or menu is empty
        ** -2 : user pressed B to exit the menu
        ** >= 0 : user choice (irrelevant on menu using folders, so prefer using an overload returning the object)
        *******************************************/
        // This return a menuEvent value
        int     ProcessEvent(Event &event, MenuItem **userchoice);
        bool    drawFooter;
    private:
        MenuFolderImpl  *_folder;

        Clock           _input;
        Clock           _scrollClock;

        IconCallback    _iconCallback;

        int             _selector;
        float           _scrollOffset;
    };
}

#endif
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

        Menu(std::string title, IconCallback iconCallback = nullptr);
        Menu(MenuFolderImpl *folder, IconCallback iconCallback = nullptr);
        ~Menu(void);

        void    Append(MenuItem *item);
        void    Draw(void);
        void    Open(MenuFolderImpl *folder);
        //void    Update(Time delta);

        /*
        ** Return value: 
        ** -1 : error or menu is empty
        ** -2 : user pressed B to exit the menu
        ** >= 0 : user choice (irrelevant on menu using folders, so prefer using an overload returning the object)
        *******************************************/
        int     ProcessEvent(Event &event, std::string &userchoice);
        // This return a menuEvent value
        int     ProcessEvent(Event &event, MenuItem **userchoice);
        int     ProcessEvent(Event &event);

    private:
        MenuFolderImpl  *_folder;
       // IntRect     _background;
       // IntRect     _border;
        Clock       _input;
        IconCallback    _iconCallback;

        int         _selector;
    };
}

#endif
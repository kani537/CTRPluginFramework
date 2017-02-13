#ifndef CTRPLUGINFRAMEWORK_MENU_HPP
#define CTRPLUGINFRAMEWORK_MENU_HPP

#include "types.h"
#include "CTRPluginFrameworkImpl/Menu.hpp"
#include "CTRPluginFramework/Events.hpp"
#include "CTRPluginFramework/Vector.hpp"
#include "CTRPluginFramework/Rect.hpp"
#include "CTRPluginFramework/Clock.hpp"

#include "CTRPluginFramework/Time.hpp"

namespace CTRPluginFramework
{
    class Menu
    {
    public:

        Menu(std::string title);
        Menu(MenuFolderImpl *folder);
        ~Menu(void);

        void    Append(MenuItem *item);
        void    Draw(void);
        //void    Update(Time delta);

        /*
        ** Return value: 
        ** -1 : error or menu is empty
        ** -2 : user pressed B to exit the menu
        ** >= 0 : user choice (irrelevant on menu using folders, so prefer using an overload returning the object)
        *******************************************/
        int     ProcessEvent(Event &event, std::string &userchoice);
        int     ProcessEvent(Event &event, MenuItem **userchoice);
        int     ProcessEvent(Event &event);

    private:
        MenuFolderImpl  *_folder;
        IntRect     _background;
        IntRect     _border;
        Clock       _input;

        int         _selector;
    };
}

#endif
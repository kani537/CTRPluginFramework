#ifndef CTRPLUGINFRAMEWORK_MENU_HPP
#define CTRPLUGINFRAMEWORK_MENU_HPP

#include "types.h"
#include "CTRPluginFramework/MenuItem.hpp"
#include "CTRPluginFramework/MenuFolder.hpp"
#include "CTRPluginFramework/Events.hpp"
#include "CTRPluginFramework/Vector.hpp"
#include "CTRPluginFramework/Rect.hpp"

#include "CTRPluginFramework/Time.hpp"

namespace CTRPluginFramework
{
    class Menu
    {
    public:
        Menu(void);
        ~Menu(void);

        void    Append(MenuItem *item);
        void    Draw(void);
        //void    Update(Time delta);
        int     ProcessEvent(Event &event, std::string &userchoice);
        int     ProcessEvent(Event &event);

    private:
        MenuFolder  *_folder;
        IntRect     _background;
        IntRect     _border;

        int         _selector;
    };
}

#endif
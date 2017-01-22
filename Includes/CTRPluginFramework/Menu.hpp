#ifndef CTRPLUGINFRAMEWORK_MENU_HPP
#define CTRPLUGINFRAMEWORK_MENU_HPP

#include "CTRPluginFramework.hpp"
#include "MenuItem.hpp"
#include "MenuFolder.hpp"
#include "MenuEntry.hpp"

namespace CTRPluginFramework
{
    class Menu
    {
        struct MenuFlags
        {
            bool showFPS : 1;
        };

    public:

        Menu(std::string name = "Cheats", std::string note = "");
        ~Menu(void);

        void    Append(MenuItem *item);
        int     Run(void);
    private:

        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _ProcessEvent(Event &event);
        void    _Update(Time delta);

        MenuFolder      *_folder;
        MenuFolder      *_starred;
        bool            _isOpen;

        int             _selector;
        Clock           _scrollClock;

    };
}

#endif
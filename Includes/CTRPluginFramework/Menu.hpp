#ifndef CTRPLUGINFRAMEWORK_MENU_HPP
#define CTRPLUGINFRAMEWORK_MENU_HPP

#include "MenuItem.hpp"
#include "MenuEntry.hpp"
#include "MenuFolder.hpp"
#include "EventManager.hpp"

namespace CTRPluginFramework
{
    class Menu
    {
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
        bool            _isOpen;
    };
}

#endif
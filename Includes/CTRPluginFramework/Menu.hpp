#ifndef CTRPLUGINFRAMEWORK_MENU_HPP
#define CTRPLUGINFRAMEWORK_MENU_HPP

#include "MenuItem.hpp"
#include "EventManager.hpp"

namespace CTRPluginFramework
{
    class Menu
    {
    public:

        Menu();
        ~Menu();

        bool    Append(MenuItem &item);
        bool    Insert(MenuItem &item, int index);

        int     Run(void);
    private:

        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _Update(Time delta);

        EventManager    _manager;
        bool            _isOpen;
    };
}

#endif
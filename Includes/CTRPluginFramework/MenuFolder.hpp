#ifndef CTRPLUGINFRAMEWORK_MENUFOLDER_HPP
#define CTRPLUGINFRAMEWORK_MENUFOLDER_HPP

#include "types.h"
#include "MenuItem.hpp"
#include <vector>

namespace CTRPluginFramework
{
    class MenuFolder : public MenuItem
    {
    public:
        MenuFolder(std::string name, std::string note = "");
        ~MenuFolder();

        void    Append(MenuItem *item);
        u32     ItemsCount(void);

    private:
        friend class Menu;

        // Private methods
        void            _Open(MenuFolder *parent);
        MenuFolder      *_Close(void);

        // Private members
        MenuFolder              *_parent;
        std::vector<MenuItem *>   _items;

    };
}

#endif
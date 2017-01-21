#ifndef CTRPLUGINFRAMEWORK_MENUFOLDER_HPP
#define CTRPLUGINFRAMEWORK_MENUFOLDER_HPP

#include "types.h"
#include "MenuItem.hpp"
#include <list>

namespace CTRPluginFramework
{
    class MenuFolder : MenuItem
    {
    public:
        MenuFolder(std::string name, std::string note = "");
        ~MenuFolder();

        bool    Append(MenuItem &item);
        bool    Insert(MenuItem &item, int index);

        u32     GetLength(void);
    private:
        friend class Menu;
        void            Open(MenuFolder *parent);
        MenuFolder      *Close(void);

        MenuFolder      *_parent;

        std::list<MenuItem>   _items;

    };
}

#endif
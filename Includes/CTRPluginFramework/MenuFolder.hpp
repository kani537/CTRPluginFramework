#ifndef CTRPLUGINFRAMEWORK_MENUFOLDER_HPP
#define CTRPLUGINFRAMEWORK_MENUFOLDER_HPP

#include "CTRPluginFramework/MenuItem.hpp"
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
        friend class PluginMenu;

        // Private methods
        void            _Open(MenuFolder *parent, int position, bool starMode);
        MenuFolder      *_Close(int &position, bool starMode);

        // Private members
        MenuFolder              *_parent[2];
        std::vector<MenuItem *>   _items;
        int                     _position[2];

    };
}

#endif
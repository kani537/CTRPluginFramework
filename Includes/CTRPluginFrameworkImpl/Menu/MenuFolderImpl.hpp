#ifndef CTRPLUGINFRAMEWORK_MENUFOLDER_HPP
#define CTRPLUGINFRAMEWORK_MENUFOLDER_HPP

#include "CTRPluginFrameworkImpl/Menu/MenuItem.hpp"

#include <vector>
#include <string>

namespace CTRPluginFramework
{
    class Menu;
    class MenuFolder : public MenuItem
    {
    public:
        MenuFolderImpl(std::string name, std::string note = "");
        ~MenuFolderImpl();

        void    Append(MenuItem *item);
        u32     ItemsCount(void);

    private:
        friend class PluginMenuImpl;
        friend class Menu;

        // Private methods
        void            _Open(MenuFolderImpl *parent, int position, bool starMode = false);
        MenuFolderImpl      *_Close(int &position, bool starMode = false);

        // Private members
        MenuFolderImpl              *_parent[2];
        std::vector<MenuItem *>     _items;
        int                         _position[2];

    };
}

#endif

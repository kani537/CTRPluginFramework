#ifndef CTRPLUGINFRAMEWORKIMPL_MENUFOLDERIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_MENUFOLDERIMPL_HPP

#include "types.h"

#include "CTRPluginFrameworkImpl/Menu/MenuItem.hpp"
#include <vector>
#include <string>

namespace CTRPluginFramework
{
    class Menu;
    class MenuFolderImpl : public MenuItem
    {
    public:
        MenuFolderImpl(const std::string &name, const std::string &note = "");
        ~MenuFolderImpl();

        void        Append(MenuItem *item, bool isStar = false);
        u32         ItemsCount(void) const;
        MenuItem    *GetItem(u32 uid);
        void        DisableAll(void);

        MenuItem    *operator[](int index);
        bool        HasParent(void);
    private:
        friend class MenuItem;
        friend class PluginMenuImpl;
        friend class PluginMenuHome;
        friend class FreeCheats;
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

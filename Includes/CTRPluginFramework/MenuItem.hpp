#ifndef CTRPLUGINFRAMEWORK_MENUITEM_HPP
#define CTRPLUGINFRAMEWORK_MENUITEM_HPP

#include <string>

namespace CTRPluginFramework
{
    enum MenuType
    {
        Folder,
        Entry
    };

    class MenuItem
    {
    public:
        MenuItem(MenuType type) : _type(type) 
        {}

        std::string     name;
        std::string     infos;
    private:
        friend class Menu;
        MenuType  _type;
    };
}

#endif
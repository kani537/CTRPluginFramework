#include "MenuFolder.hpp"

namespace CTRPluginFramework
{
    MenuFolder::MenuFolder(std::string name, std::string note) :
    MenuItem(MenuType::Folder)
    {
        this->name = name;
        this->note = note;
    }

    MenuFolder::~MenuFolder()
    {

    }

    void    MenuFolder::Append(MenuItem *item)
    {
        _items.push_back(item);

    }

    u32    MenuFolder::ItemsCount(void)
    {
        return (_items.size());
    }

    //#######################################################################

    void    MenuFolder::_Open(MenuFolder *parent)
    {
        _parent = parent;
    }

    MenuFolder  *MenuFolder::_Close(void)
    {
        return (_parent);
    }

}
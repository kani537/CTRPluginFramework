#include "types.h"

#include "CTRPluginFramework/MenuItem.hpp"
#include "CTRPluginFramework/MenuEntry.hpp"
#include "CTRPluginFramework/MenuFolder.hpp"

namespace CTRPluginFramework
{
    MenuFolder::MenuFolder(std::string name, std::string note) :
    MenuItem(MenuType::Folder)
    {
        this->name = name;
        this->note = note;
        this->_position = -1;
        this->_parent = nullptr;
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

    void    MenuFolder::_Open(MenuFolder *parent, int position)
    {
        _parent = parent;
        _position = position;
    }

    MenuFolder  *MenuFolder::_Close(int &position)
    {
        if (_parent != nullptr && position != -1)
            position = _position;
        return (_parent);
    }

}
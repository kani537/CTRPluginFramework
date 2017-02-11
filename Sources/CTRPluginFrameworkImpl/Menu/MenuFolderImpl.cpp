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
        this->_position[0] = -1;
        this->_position[1] = -1;
        this->_parent[0] = nullptr;        
        this->_parent[1] = nullptr;
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

    void    MenuFolder::_Open(MenuFolder *parent, int position, bool starMode)
    {
        int index = starMode ? 1 : 0;
        _parent[index] = parent;
        _position[index] = position;
    }

    MenuFolder  *MenuFolder::_Close(int &position, bool starMode)
    {
        int index = starMode ? 1 : 0;
        if (_parent[index] != nullptr && _position[index] != -1)
            position = _position[index];
        return (_parent[index]);
    }

}
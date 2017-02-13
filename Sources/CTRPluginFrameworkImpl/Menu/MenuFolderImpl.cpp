#include "types.h"

#include "CTRPluginFrameworkImpl/Menu.hpp"

namespace CTRPluginFramework
{
    MenuFolderImpl::MenuFolderImpl(std::string name, std::string note) :
    MenuItem(MenuType::Folder)
    {
        this->name = name;
        this->note = note;
        this->_position[0] = -1;
        this->_position[1] = -1;
        this->_parent[0] = nullptr;        
        this->_parent[1] = nullptr;
    }

    MenuFolderImpl::~MenuFolderImpl()
    {

    }

    void    MenuFolderImpl::Append(MenuItem *item)
    {
        _items.push_back(item);

    }

    u32    MenuFolderImpl::ItemsCount(void)
    {
        return (_items.size());
    }

    //#######################################################################

    void    MenuFolderImpl::_Open(MenuFolderImpl *parent, int position, bool starMode)
    {
        int index = starMode ? 1 : 0;
        _parent[index] = parent;
        _position[index] = position;
    }

    MenuFolderImpl  *MenuFolderImpl::_Close(int &position, bool starMode)
    {
        int index = starMode ? 1 : 0;
        if (_parent[index] != nullptr && _position[index] != -1)
            position = _position[index];
        return (_parent[index]);
    }

}
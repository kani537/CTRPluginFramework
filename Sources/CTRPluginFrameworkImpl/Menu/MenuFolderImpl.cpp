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

    void    MenuFolderImpl::Append(MenuItem *item, bool isStar)
    {
        if (!isStar)
        {
            item->_container = this;
            item->_index = _items.size();
        }        
        _items.push_back(item);

    }

    u32    MenuFolderImpl::ItemsCount(void) const
    {
        return (_items.size());
    }

    MenuItem *MenuFolderImpl::GetItem(u32 uid)
    {
        for (MenuItem *item : _items)
        {
            if (item->_uid == uid)
                return (item);

            if (item->_type == MenuType::Folder)
            {
                MenuItem * i = reinterpret_cast<MenuFolderImpl *>(item)->GetItem(uid);

                if (i != nullptr)
                    return (i);
            }
        }

        return (nullptr);
    }

    void    MenuFolderImpl::DisableAll(void)
    {
        for (MenuItem *item : _items)
        {
            if (item->IsEntry() || item->IsFreeCheat())
                reinterpret_cast<MenuEntryImpl *>(item)->Disable();
            if (item->IsFolder())
                reinterpret_cast<MenuFolderImpl *>(item)->DisableAll();
        }
    }

    MenuItem* MenuFolderImpl::operator[](int index)
    {
        if (index >= _items.size())
            return (nullptr);

        return (_items[index]);
    }

    bool MenuFolderImpl::HasParent()
    {
        return (_parent[0] != nullptr || _parent[1] != nullptr);
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
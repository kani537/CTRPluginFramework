#include "CTRPluginFramework/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu.hpp"


namespace CTRPluginFramework
{
    MenuFolder::MenuFolder(const std::string &name, const std::string &note) :
        _item(new MenuFolderImpl(this, name, note))
    {

    }

    MenuFolder::MenuFolder(const std::string& name, const std::vector<MenuEntry*>& entries) :
        _item(new MenuFolderImpl(this, name))
    {
        for (MenuEntry *entry : entries)
            Append(entry);
    }

    MenuFolder::MenuFolder(const std::string& name, const std::string& note, const std::vector<MenuEntry*>& entries) :
        _item(new MenuFolderImpl(this, name, note))
    {
        for (MenuEntry *entry : entries)
            Append(entry);
    }

    MenuFolder::~MenuFolder()
    {
    }

    void    MenuFolder::Hide(void) const
    {
        _item->Hide();
    }

    void    MenuFolder::Show(void) const
    {
        _item->Show();
    }

    bool    MenuFolder::IsVisible() const
    {
        return(_item->IsVisible());
    }


    void    MenuFolder::Append(MenuEntry *item) const
    {
        MenuEntryImpl *entry = item->_item.get();

        _item->Append(entry);
    }

    void    MenuFolder::Append(MenuFolder *item) const
    {
        MenuFolderImpl *folder = item->_item.get();

        _item->Append(folder);
    }

    std::vector<MenuEntry *>    MenuFolder::GetEntryList(void) const
    {
        return (_item->GetEntryList());
    }

    std::vector<MenuFolder *>   MenuFolder::GetFolderList(void) const
    {
        return (_item->GetFolderList());
    }

    u32    MenuFolder::ItemsCount(void) const
    {
        return (_item->ItemsCount());
    }

    MenuFolder    *MenuFolder::operator += (const MenuEntry *item)
    {
        MenuEntryImpl *entry = item->_item.get();

        _item->Append(entry);

        return (this);
    }

    MenuFolder  *MenuFolder::operator+=(const MenuFolder *folder)
    {
        MenuFolderImpl *f = folder->_item.get();

        _item->Append(f);
        return (this);
    }
}

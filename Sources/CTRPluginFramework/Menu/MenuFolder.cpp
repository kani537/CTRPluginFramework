#include "CTRPluginFramework/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu.hpp"


namespace CTRPluginFramework
{
    MenuFolder::MenuFolder(std::string name, std::string note) :
        _item(new MenuFolderImpl(name, note))
    {

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

    u32    MenuFolder::ItemsCount(void) const
    {
        return (_item->ItemsCount());
    }
}
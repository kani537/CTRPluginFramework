#include "CTRPluginFramework/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu.hpp"

namespace CTRPluginFramework
{
    PluginMenu::PluginMenu(std::string name, std::string note) :
        _menu(new PluginMenuImpl(name, note)) 
    {

    }

    void    PluginMenu::Append(MenuEntry *item)
    {
        MenuEntryImpl *entry = item->_item;
        _menu->Append(entry);
    }

    void    PluginMenu::Append(MenuFolder *item)
    {
        MenuFolderImpl *folder = item->_item;
        _menu->Append(folder);
    }

    int    PluginMenu::Run(void)
    {
       return (_menu->Run());
    }
}

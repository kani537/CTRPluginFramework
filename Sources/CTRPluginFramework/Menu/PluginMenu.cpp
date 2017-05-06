#include "CTRPluginFramework/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu.hpp"

namespace CTRPluginFramework
{
    PluginMenu::PluginMenu(std::string name, std::string note) :
        _menu(new PluginMenuImpl(name, note)) 
    {

    }

    void    PluginMenu::Append(MenuEntry *item) const
    {
        MenuEntryImpl *entry = item->_item;
        _menu->Append(entry);
    }

    void    PluginMenu::Append(MenuFolder *item) const
    {
        MenuFolderImpl *folder = item->_item;
        _menu->Append(folder);
    }

    void    PluginMenu::Callback(CallbackPointer callback) const
    {
        _menu->Callback(callback);
    }

    int    PluginMenu::Run(void) const
    {
       return (_menu->Run());
    }

    void    PluginMenu::SetSearchButtonState(bool isEnabled) const
    {
        _menu->TriggerSearch(isEnabled);
    }

    void    PluginMenu::SetActionReplayButtonState(bool isEnabled) const
    {
        return;
        _menu->TriggerActionReplay(isEnabled);
    }
}

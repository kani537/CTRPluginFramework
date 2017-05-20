#include "CTRPluginFramework/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuImpl.hpp"

namespace CTRPluginFramework
{
    static const PluginMenu   *g_runningInstance = nullptr;

    PluginMenu::PluginMenu(std::string name, std::string about) :
        _menu(new PluginMenuImpl(name, about)) 
    {

    }

    PluginMenu::PluginMenu(std::string name, void *about, DecipherPointer func)
    {
        std::string aboutStr = "";
        func(aboutStr, about);

        _menu = std::unique_ptr<PluginMenuImpl>(new PluginMenuImpl(name, aboutStr));
    }

    PluginMenu::~PluginMenu(void)
    {
    }

    void    PluginMenu::Append(MenuEntry *item) const
    {
        if (item == nullptr)
            return;

        MenuEntryImpl *entry = item->_item.get();
        _menu->Append(entry);
    }

    void    PluginMenu::Append(MenuFolder *item) const
    {
        if (item == nullptr)
            return;

        MenuFolderImpl *folder = item->_item.get();
        _menu->Append(folder);
    }

    void    PluginMenu::Callback(CallbackPointer callback) const
    {
        if (callback != nullptr)
            _menu->Callback(callback);
    }

    int    PluginMenu::Run(void) const
    {
        g_runningInstance = this;

        int ret = _menu->Run();

        g_runningInstance = nullptr;

        return (ret);
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

    bool    PluginMenu::IsOpen(void)
    {
        return (_menu->IsOpen());
    }

    bool    PluginMenu::WasOpened(void)
    {
        return (_menu->WasOpened());
    }

    PluginMenu  &PluginMenu::GetRunningInstance(void)
    {
        return (const_cast<PluginMenu &>(*g_runningInstance));
    }
}

#include "types.h"

#include "CTRPluginFramework/Menu/MenuEntry.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"

namespace CTRPluginFramework
{
    MenuEntry::MenuEntry(std::string name, std::string note) : 
        _item(new MenuEntryImpl(name, note, this))
    {   
    }
    
    MenuEntry::MenuEntry(std::string name, FuncPointer func, std::string note) : 
        _item(new MenuEntryImpl(name, func, note, this))
    {

    }

    void    MenuEntry::Disable(void)
    {
        _item->Disable();
    }

    void    MenuEntry::SetRadio(int id)
    {
        _item->SetRadio(id);
    }

    void    MenuEntry::SetArg(void *arg)
    {
        _item->SetArg(arg);
    }

    void    *MenuEntry::GetArg(void)
    {
        return (_item->GetArg());
    }

    bool    MenuEntry::WasJustActivated(void)
    {
        return (_item->WasJustActivated());
    }

    bool    MenuEntry::IsActivated(void)
    {
        return (_item->IsActivated());
    }

    void    MenuEntry::SetGameFunc(FuncPointer func)
    {
        _item->GameFunc = func;
    }

    void    MenuEntry::SetMenuFunc(FuncPointer func)
    {
        _item->MenuFunc = func;
    }

    std::string &MenuEntry::Name(void)
    {
        return (_item->name);
    }

    std::string &MenuEntry::Note(void)
    {
        return (_item->note);
    }
}

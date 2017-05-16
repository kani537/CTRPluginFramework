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

    MenuEntry::MenuEntry(std::string name, FuncPointer GameFunc, FuncPointer MenuFunc, std::string note) :
        _item(new MenuEntryImpl(name, GameFunc, note, this))
    {
        _item->MenuFunc = MenuFunc;
    }

    MenuEntry::MenuEntry(int radioId, std::string name, FuncPointer func, std::string note) : 
        _item(new MenuEntryImpl(name, func, note, this))
    {
        _item->SetRadio(radioId);
    }

    MenuEntry::MenuEntry(int radioGroup, std::string name, FuncPointer GameFunc, FuncPointer MenuFunc, std::string note) :
        _item(new MenuEntryImpl(name, GameFunc, note, this))
    {
        _item->SetRadio(radioGroup);
        _item->MenuFunc = MenuFunc;
    }

    MenuEntry::~MenuEntry()
    {
    }

    void    MenuEntry::Disable(void) const
    {
        _item->Disable();
    }

    void    MenuEntry::Hide(void) const
    {
        _item->Hide();
    }

    void    MenuEntry::Show(void) const
    {
        _item->Show();
    }

    void    MenuEntry::SetRadio(int id) const
    {
        _item->SetRadio(id);
    }

    void    MenuEntry::SetArg(void *arg) const
    {
        _item->SetArg(arg);
    }

    void    *MenuEntry::GetArg(void) const
    {
        return (_item->GetArg());
    }

    bool    MenuEntry::WasJustActivated(void) const
    {
        return (_item->WasJustActivated());
    }

    bool    MenuEntry::IsActivated(void) const
    {
        return (_item->IsActivated());
    }

    bool    MenuEntry::IsVisible() const
    {
        return (_item->IsVisible());
    }

    void    MenuEntry::SetGameFunc(FuncPointer func) const
    {
        _item->GameFunc = func;
    }

    void    MenuEntry::SetMenuFunc(FuncPointer func) const
    {
        _item->MenuFunc = func;
    }

    std::string &MenuEntry::Name(void) const
    {
        return (_item->name);
    }

    std::string &MenuEntry::Note(void) const
    {
        return (_item->note);
    }
}

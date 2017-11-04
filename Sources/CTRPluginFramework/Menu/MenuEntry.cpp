#include "types.h"

#include "CTRPluginFramework/Menu/MenuEntry.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"

namespace CTRPluginFramework
{
    MenuEntry::MenuEntry(const std::string &name, const std::string &note) : 
        _item(new MenuEntryImpl(name, note, this)),
        Hotkeys(this)
    {   
    }
    
    MenuEntry::MenuEntry(const std::string &name, FuncPointer func, const std::string &note) : 
        _item(new MenuEntryImpl(name, func, note, this)),
        Hotkeys(this)
    {

    }

    MenuEntry::MenuEntry(const std::string &name, FuncPointer GameFunc, FuncPointer MenuFunc, const std::string &note) :
        _item(new MenuEntryImpl(name, GameFunc, note, this)),
        Hotkeys(this)
    {
        _item->MenuFunc = MenuFunc;
    }

    MenuEntry::MenuEntry(int radioId, const std::string &name, FuncPointer func, const std::string &note) : 
        _item(new MenuEntryImpl(name, func, note, this)),
        Hotkeys(this)
    {
        _item->SetRadio(radioId);
    }

    MenuEntry::MenuEntry(int radioGroup, const std::string &name, FuncPointer GameFunc, FuncPointer MenuFunc, const std::string &note) :
        _item(new MenuEntryImpl(name, GameFunc, note, this)),
        Hotkeys(this)
    {
        _item->SetRadio(radioGroup);
        _item->MenuFunc = MenuFunc;
    }

    MenuEntry::~MenuEntry()
    {
    }

    void    MenuEntry::Enable(void) const
    {
        _item->Enable();
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

    void    MenuEntry::UseTopSeparator(bool useSeparator, Separator type) const
    {
        _item->Flags.useSeparatorBefore = useSeparator;
        _item->Flags.useStippledLineForBefore = type == Separator::Stippled;
    }

    void    MenuEntry::UseBottomSeparator(bool useSeparator, Separator type) const
    {
        _item->Flags.useSeparatorAfter = useSeparator;
        _item->Flags.useStippledLineForAfter = type == Separator::Stippled;
    }

    void    MenuEntry::SetGameFunc(FuncPointer func) const
    {
        _item->GameFunc = func;
    }

    void    MenuEntry::SetMenuFunc(FuncPointer func) const
    {
        _item->MenuFunc = func;
    }

    void MenuEntry::RefreshNote() const
    {
        _item->NoteChanged();
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

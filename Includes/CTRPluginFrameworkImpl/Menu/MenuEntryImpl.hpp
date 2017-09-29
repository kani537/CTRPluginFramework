#ifndef CTRPLUGINFRAMEWORKIMPL_MENUENTRYIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_MENUENTRYIMPL_HPP

#include "CTRPluginFrameworkImpl/Menu/MenuItem.hpp"
#include "CTRPluginFramework/Menu/MenuEntry.hpp"
#include <string>

namespace CTRPluginFramework
{
    class MenuEntryImpl;
    
    using FuncPointer = void (*)(MenuEntry*);
    
    class MenuEntryImpl : public MenuItem
    {
        struct Flags
        {
            bool  state : 1;
            bool  justChanged : 1;
            bool  isRadio : 1;
        };

    public:
        MenuEntryImpl(const std::string &name, const std::string &note = "", MenuEntry *owner = nullptr);
        MenuEntryImpl(const std::string &name, FuncPointer func, const std::string &note = "", MenuEntry *owner = nullptr);
        virtual ~MenuEntryImpl();

        // Enable the entry
        void    Enable(void);
        // Disable the entry
        void    Disable(void);
        // Set the entry as radio, an ID must be provided
        void    SetRadio(int id);        
        // Set an argument for the entry
        void    SetArg(void *arg);
        // Get the argument
        void    *GetArg(void) const;
        // Return if the entry just got activated
        bool    WasJustActivated(void) const;
        // Return if the entry is activated
        bool    IsActivated(void) const;
        MenuEntry *AsMenuEntry(void) const;

        // Public members
        FuncPointer     GameFunc;
        FuncPointer     MenuFunc;

        virtual std::string &GetNote(void) override;

        std::string     Note2;
    protected:
        friend class PluginMenuHome;
        friend class PluginMenuExecuteLoop;
        // Functions used by the menu
        bool    _TriggerState(void);
        bool    _MustBeRemoved(void) const;
        bool    _Execute(void);
        int     _executeIndex;
        MenuEntry *_owner;

        Flags       _flags;
        int         _radioId;
        void        *_arg;
    };
}

#endif
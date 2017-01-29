#ifndef CTRPLUGINFRAMEWORK_MENUENTRY_HPP
#define CTRPLUGINFRAMEWORK_MENUENTRY_HPP

#include "CTRPluginFramework/MenuItem.hpp"

namespace CTRPluginFramework
{
    class MenuEntry;
    
    using FuncPointer = void (*)(MenuEntry*);
    
    class MenuEntry : public MenuItem
    {
        struct Flags
        {
            bool  state : 1;
            bool  justChanged : 1;
            bool  isRadio : 1;
        };

    public:
        MenuEntry(std::string name, std::string note = "");
        MenuEntry(std::string name, FuncPointer func, std::string note = "");
        ~MenuEntry();

        // Disable the entry
        void    Disable(void);
        // Set the entry as radio, an ID must be provided
        void    SetRadio(int id);        
        // Set an argument for the entry
        void    SetArg(void *arg);
        // Get the argument
        void    *GetArg(void);
        // Return if the entry just got activated
        bool    WasJustActivated(void);
        // Return if the entry is activated
        bool    IsActivated(void);

        // Public members
        FuncPointer     GameFunc;
        FuncPointer     MenuFunc;

    private:
        friend class Menu;

        // Functions used by the menu
        bool    _TriggerState(void);
        bool    _MustBeRemoved(void);
        bool    _Execute(void);
        int     _executeIndex;

        Flags       _flags;
        int         _radioId;
        void        *_arg;
    };
}

#endif
#ifndef CTRPLUGINFRAMEWORK_MENUENTRY_HPP
#define CTRPLUGINFRAMEWORK_MENUENTRY_HPP

namespace CTRPluginFramework
{
    class MenuEntry : MenuItem
    {
        struct Flags
        {
            bool  state : 1;
            bool  justChanged : 1;
            bool  isRadio : 1;
            bool  isStarred : 1;  
            bool  isImmediate : 1;
        };

    public:
        MenuEntry(std::string name, std::string note = "");
        MenuEntry(std::string name, FuncPointer func, std::string note = "");
        ~MenuEntry();

        // Disable the entry
        void    Disable(void);
        // Set the entry as radio, an ID must be provided
        void    SetRadio(int id);        
        // Set it as an immediate entry (which will be enabled on the menu ONLY)
        void    SetImmediate(void);
        // Set an argument for the entry
        void    SetArg(void *arg);
        // Get the argument
        void    *GetArg(void);
        // Return if the entry just got activated
        bool    WasJustActivated(void);
        // Return if the entry is activated
        bool    IsActivated(void);

        // Public members
        FuncPointer     Func;

    private:
        friend class Menu;

        // Functions used by the menu
        void    Activate(void);
        void    Deactivate(void);
        void    Star(void);
        void    UnStar(void);
        void    Execute(void);

        Flags       _flags;
        int         _radioId;
    };
}

#endif
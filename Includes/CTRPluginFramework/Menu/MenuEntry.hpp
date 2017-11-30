#ifndef CTRPLUGINFRAMEWORK_MENUENTRY_HPP
#define CTRPLUGINFRAMEWORK_MENUENTRY_HPP

#include "CTRPluginFramework/Menu/MenuEntryHotkeys.hpp"

#include <string>
#include <memory>

namespace CTRPluginFramework
{
#ifndef SEPARATOR_TYPE
#define SEPARATOR_TYPE
    enum class Separator
    {
        Filled,
        Stippled
    };
#endif

    class MenuEntryImpl;
    class MenuEntry
    {
        using FuncPointer = void(*)(MenuEntry*);
    public:

        explicit MenuEntry(const std::string &name, const std::string &note = "");
        MenuEntry(const std::string &name, FuncPointer gameFunc, const std::string &note = "");
        MenuEntry(const std::string &name, FuncPointer gameFunc, FuncPointer menuFunc, const std::string &note = "");
        MenuEntry(int radioGroup, const std::string &name, FuncPointer gameFunc, const std::string &note = "");
        MenuEntry(int radioGroup, const std::string &name, FuncPointer gameFunc, FuncPointer menuFunc, const std::string &note = "");
        ~MenuEntry();

        // Enable the entry
        void    Enable(void) const;
        // Disable the entry
        void    Disable(void) const;
        // Hide the entry from the menu. Will disable it too
        void    Hide(void) const;
        // Reinsert an entry previously hidden in the menu
        void    Show(void) const;
        // Set the entry as radio, an ID must be provided
        void    SetRadio(int id) const;
        // Set an argument for the entry
        void    SetArg(void *arg) const;
        // Get the argument of the entry
        void    *GetArg(void) const;
        // Return if the entry just got activated
        bool    WasJustActivated(void) const;
        // Return if the entry is activated
        bool    IsActivated(void) const;
        // Return if the entry is visible in the menu
        bool    IsVisible(void) const;

        /**
        * \brief Set if this entry must display a separator on top of the entry
        * \param useSeparator pass true if the separator must be displayed, false otherwise
        * \param type Type of separator to display
        */
        void    UseTopSeparator(bool useSeparator, Separator type = Separator::Filled) const;

        /**
        * \brief Set if this entry must display a separator at the bottom of the entry
        * \param useSeparator pass true if the separator must be displayed, false otherwise
        * \param type Type of separator to display
        */
        void    UseBottomSeparator(bool useSeparator, Separator type = Separator::Filled) const;
        
        /**
         * \brief Set if the entry can be selected in the menu or not.\n
         * If the entry is Activated and the state is set to unselectable, the entry will be disabled
         * \param canBeSelected 
         */
        void    CanBeSelected(bool canBeSelected) const;

        void    SetGameFunc(FuncPointer func) const;
        void    SetMenuFunc(FuncPointer func) const;
        void    RefreshNote(void) const;

        std::string &Name(void) const;
        std::string &Note(void) const;

        HotkeyManager   Hotkeys;

    private:
        friend class MenuFolder;
        friend class PluginMenu;
        std::unique_ptr<MenuEntryImpl>  _item;
    };
}

#endif
#ifndef CTRPLUGINFRAMEWORKIMPL_MENUENTRYTOOLS_HPP
#define CTRPLUGINFRAMEWORKIMPL_MENUENTRYTOOLS_HPP

#include "MenuEntryImpl.hpp"
#include <string>


namespace CTRPluginFramework
{
    class MenuEntryTools : public MenuEntryImpl
    {
        using IconCallback = int(*)(int, int);
        using FuncPointer = void(*)(void);
        using FuncPointerA = void(*)(MenuEntryTools *);
    public:
        MenuEntryTools(const std::string &text, FuncPointer func, IconCallback icon, void *arg = nullptr);
        MenuEntryTools(const std::string &text, FuncPointer func, IconCallback icon, const std::string &note);
        MenuEntryTools(const std::string &text, FuncPointer func, bool useCheckBox, bool isEnabled = false, const std::string &note = "");
        MenuEntryTools(const std::string &text, FuncPointerA func, bool useCheckBox, bool isEnabled = false, const std::string &note = "");

        ~MenuEntryTools() {}

        void    TriggerState(void);

        IconCallback    Icon;
        FuncPointer     Func;
        FuncPointerA    FuncArg;
        bool            UseCheckBox;
    };
}

#endif

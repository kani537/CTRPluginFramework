#include "CTRPluginFrameworkImpl/Menu/MenuEntryTools.hpp"

namespace CTRPluginFramework
{
    MenuEntryTools::MenuEntryTools(const std::string& text, FuncPointer func, IconCallback icon, void *arg) :
        MenuEntryImpl(text),
        Icon(icon),
        Func(func),
        FuncArg(nullptr),
        UseCheckBox(false)
    {
        _type = EntryTools;
        _arg = arg;
    }

    MenuEntryTools::MenuEntryTools(const std::string& text, FuncPointer func, IconCallback icon, const std::string& note) :
        MenuEntryImpl(text, note),
        Icon(icon),
        Func(func),
        FuncArg(nullptr),
        UseCheckBox(false)
    {
        _type = EntryTools;
    }

    MenuEntryTools::MenuEntryTools(const std::string& text, FuncPointer func, bool useCheckBox, bool isEnabled, const std::string& note) :
        MenuEntryImpl(text, note),
        Icon(nullptr),
        Func(func),
        FuncArg(nullptr),
        UseCheckBox(useCheckBox)
    {
        _type = EntryTools;
        if (isEnabled)
            _TriggerState();
    }

    MenuEntryTools::MenuEntryTools(const std::string& text, FuncPointerA func, bool useCheckBox, bool isEnabled, const std::string& note) :
        MenuEntryImpl(text, note),
        Icon(nullptr),
        Func(nullptr),
        FuncArg(func),
        UseCheckBox(useCheckBox)
    {
        _type = EntryTools;
        if (isEnabled)
            _TriggerState();
    }

    void    MenuEntryTools::TriggerState(void)
    {
        _TriggerState();
    }
}

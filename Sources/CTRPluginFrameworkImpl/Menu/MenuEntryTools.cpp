#include "CTRPluginFrameworkImpl/Menu/MenuEntryTools.hpp"

namespace CTRPluginFramework
{
    MenuEntryTools::MenuEntryTools(const std::string& text, FuncPointer func, IconCallback icon, void *arg) :
        MenuEntryImpl(text),
        Icon(icon),
        Func(func),
        UseCheckBox(false)
    {
        _type = EntryTools;
        _arg = arg;
    }

    MenuEntryTools::MenuEntryTools(const std::string& text, FuncPointer func, IconCallback icon, const std::string& note) :
        MenuEntryImpl(text, note),
        Icon(icon),
        Func(func),
        UseCheckBox(false)
    {
        _type = EntryTools;
    }

    MenuEntryTools::MenuEntryTools(const std::string& text, FuncPointer func, bool useCheckBox, bool isEnabled, const std::string& note) :
        MenuEntryImpl(text, note),
        Icon(nullptr),
        Func(func),
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

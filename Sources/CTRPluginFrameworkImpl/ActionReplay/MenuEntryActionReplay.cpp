#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/Utils/StringExtensions.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/MenuEntryActionReplay.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARHandler.hpp"

namespace CTRPluginFramework
{
    static void    ActionReplay_ExecuteCode(MenuEntryImpl *entry)
    {
        if (!entry) return;

        MenuEntryActionReplay *ar = reinterpret_cast<MenuEntryActionReplay *>(entry);

        if (!ar || ar->context.hasError || ar->context.codes.empty()) return;

        ARHandler::Execute(ar->context.codes, ar->context.storage);
    }

    MenuEntryActionReplay::MenuEntryActionReplay(const std::string &name, const std::string &note) :
        MenuEntryImpl{ name, note }
    {
        _type = ActionReplay;
        context.Clear();
        GameFunc = (FuncPointer)ActionReplay_ExecuteCode;
    }

    MenuEntryActionReplay::~MenuEntryActionReplay()
    {
    }

    void    MenuEntryActionReplay::Update()
    {
        if (!name.empty())
        {
            if (name[0] == 0x1B)
                name = name.substr(4);
            if (context.hasError)
                name = Color::Red << name;
        }
    }
}

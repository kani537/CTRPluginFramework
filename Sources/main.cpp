#if __INTELLISENSE__
typedef unsigned int __SIZE_TYPE__;
typedef unsigned long __PTRDIFF_TYPE__;
//#undef __cplusplus
//#define __cplusplus 201103L
#undef __cpp_exceptions
#define __cpp_exceptions 0
#define __attribute__(q)
#define __extension__
#define __asm__(expr)
#define __builtin_labs(a) 0
#define __builtin_llabs(a) 0
#define __builtin_fabs(a) 0
#define __builtin_fabsf(a) 0
#define __builtin_fabsl(a) 0
#define __builtin_strcmp(a,b) 0
#define __builtin_strlen(a) 0
#define __builtin_memcpy(a,b) 0
#define __builtin_va_list void*
#define __builtin_va_start(a,b)
#endif

#include "Hook.hpp"
#include "3DS.h"
#include "CTRPluginFramework.hpp"
#include "CTRPluginFramework/System/FwkSettings.hpp"
#include "CTRPluginFrameworkImpl/Graphics/TextBox.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"
#include <string>

namespace CTRPluginFramework
{
    // This function is called on the plugin starts, before main
    void    PatchProcess(FwkSettings &settings)
    {
       // settings.WaitTimeToBoot = Seconds(10.f);
    }

    bool    IsHomebrew(void)
    {
        Handle *fsUser = fsGetSessionHandle();

        return (fsUser == nullptr);
    }

    MenuEntry *AddArg(void *arg, MenuEntry *entry)
    {
        if(entry != nullptr)
            entry->SetArg(arg);
        return (entry);
    }

    MenuEntry *EntryWithHotkey(MenuEntry *entry, const Hotkey &hotkey)
    {
        if (entry != nullptr)
        {
            entry->Hotkeys += hotkey;
            entry->SetArg(new std::string(entry->Name()));
            entry->Name() += " " + hotkey.ToString();
            entry->Hotkeys.OnHotkeyChangeCallback([](MenuEntry *entry, int index)
            {
                std::string *name = reinterpret_cast<std::string *>(entry->GetArg());

                entry->Name() = *name + " " + entry->Hotkeys[0].ToString();
            });
        }

        return (entry);
    }

    MenuEntry *EntryWithHotkey(MenuEntry *entry, const std::vector<Hotkey> &hotkeys)
    {
        if (entry != nullptr)
        {
            for (const Hotkey &hotkey : hotkeys)
                entry->Hotkeys += hotkey;
        }

        return (entry);
    }

#define BLANKVERSION 0

#if BLANKVERSION
    int     main(void)
    {
        //Directory::ChangeWorkingDirectory("/3ds/ntr/plugin/" + Utils::Format("%016llX/", Process::GetTitleID()));

        static const char *about = "This is a blank plugin to let you use ctrpf on multiple games without being annoyed by builtin cheats.";
        PluginMenu  *m = new PluginMenu("Blank plugin", 0, 3, 0, about);
        PluginMenu  &menu = *m;

#else
    int     main(void)
    {
        if (!System::IsLoaderNTR())
            Directory::ChangeWorkingDirectory(Utils::Format("/luma/plugins/%016llX/", Process::GetTitleID()));
        //Sleep(Seconds(5.f));
        PluginMenu  *m = new PluginMenu("Action Replay Test", 0, 1, 0);
        PluginMenu  &menu = *m;

#endif
       /* menu += []
        {
            Sleep(Milliseconds(5.f));
        }; */
        menu.SyncronizeWithFrame(true);
        // Launch menu and mainloop
        menu.Run();

        // Exit plugin
        return (0);
    }
}

#include "CTRPluginFramework.hpp"
#include "Hook.hpp"
#include "3DS.h"

namespace CTRPluginFramework
{
    // 0x00103DC4 then return to 0x00103F38
    // This function is called on the plugin starts, before main
    void    PatchProcess(void)
    {
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
    extern MenuFolder  *g_folder;

    void    LineReadTest(MenuEntry *entry);
    int     main(void)
    {
        Directory::ChangeWorkingDirectory("/plugin/game/");
        //Sleep(Seconds(5.f));
        PluginMenu  *m = new PluginMenu("Action Replay Test", 0, 0, 1);
        PluginMenu  &menu = *m;

        menu += new MenuEntry("Load cheats from file", nullptr, LineReadTest);
        menu += g_folder;

        MenuEntry *entry = new MenuEntry("Separator Before");

        entry->UseTopSeparator(true);
        menu += entry;

        menu += new MenuEntry("Padding");

        entry = new MenuEntry("Separator After");
        entry->UseBottomSeparator(true);
        menu += entry;

        menu += new MenuEntry("Padding");

        entry = new MenuEntry("Separator Before & After");
        entry->UseTopSeparator(true);
        entry->UseBottomSeparator(true);
        menu += entry;

        menu += new MenuEntry("Padding");

        MenuFolder *f = new MenuFolder("Separator Before & After");
        f->UseTopSeparator(true);
        f->UseBottomSeparator(true);
        menu += f;

        menu += new MenuEntry("Padding");

        entry = new MenuEntry("Separator After");
        entry->UseBottomSeparator(true);
        menu += entry;

        entry = new MenuEntry("Separator Before");

        entry->UseTopSeparator(true);
        menu += entry;

#endif
        menu += []
        {
            Sleep(Milliseconds(5));
        };

        // Launch menu and mainloop
        menu.Run();

        // Exit plugin
        return (0);
    }
}

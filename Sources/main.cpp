#include "CTRPluginFramework.hpp"
#include "Hook.hpp"
#include "3DS.h"
#include "CTRPluginFramework/System/FwkSettings.hpp"
#include "CTRPluginFrameworkImpl/Graphics/TextBox.hpp"

namespace CTRPluginFramework
{
    // This function is called on the plugin starts, before main
    void    PatchProcess(FwkSettings &settings)
    {
       // settings.WaitTimeToBoot = Seconds(10.f);
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

        // Add global folder Action Replay
        menu += g_folder;

        g_folder->OnOpening = [](MenuFolder &folder)
        {
            // If folder is empty
            if (g_folder->ItemsCount() == 0)
            {
                MessageBox("Info", "No cheats loaded, use Load cheats from file first")();
                // Don't open the folder
                return (false);
            }
            // Can be opened
            return (true);
        };

        menu += new MenuEntry(Utils::Format("Heap free: %08X", getMemFree()), nullptr);
        MenuEntry *entry = new MenuEntry("Separator Before");


        entry->UseTopSeparator(true);
        entry->CanBeSelected(false);
        menu += entry;

        std::string note = "Use \uE077 while pressing the hotkey(s) to move very fast.\n"
            << Color::Orange << "Be careful of the loading zone, it might put you out of bound.\n"
            << ResetColor() << "You can change the hotkey by touching the controller icon on the bottom screen.";

        auto e = [](MenuEntry *entry) {};
        menu += new MenuFolder("Movement", "",
        {
            EntryWithHotkey(new MenuEntry("MoonJump", e, "Press the hotkey to be free of the gravity."), Hotkey(Key::A, "Moonjump")),
            EntryWithHotkey(new MenuEntry("Fast Move \uE077 +", e, note), Hotkey(Key::L, "Run faster")),
            new MenuEntry("Epona have max carrots", e),
            new MenuEntry("Epona have max carrots", e),
            EntryWithHotkey(new MenuEntry("Epona MoonJump", e, "Press the hotkey to be free of the gravity."), Hotkey(Key::L | Key::A, "Epona Moonjump"))
        });

        /*TextBox tb("Lol", "mdr", IntRect(10, 10, 10, 10));

        menu += new MenuEntry(Utils::Format("TB: %08X", (u32)&tb));*/

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

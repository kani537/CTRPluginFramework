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
#include "CTRPluginFrameworkImpl/System/Heap.hpp"
#include <string>

namespace CTRPluginFramework
{
    // This function is called on the plugin starts, before main
    void    PatchProcess(FwkSettings &settings)
    {
        //settings.WaitTimeToBoot = Seconds(10.f);
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
    //MenuEntry *entry;
    //MenuEntry *ctrpfHeap;
    int     main(void)
    {
        if (!System::IsLoaderNTR())
        {
            if (*(vu32 *)0x070000FC)
                Directory::ChangeWorkingDirectory("/luma/plugins/ActionReplay/");
            else
                Directory::ChangeWorkingDirectory(Utils::Format("/luma/plugins/%016llX/", Process::GetTitleID()));
        }
        //Sleep(Seconds(5.f));
        PluginMenu  *m = new PluginMenu("Action Replay", 0, 1, 8);
        PluginMenu  &menu = *m;

      /*  entry = new MenuEntry(Utils::Format("Newlib MemFree: %08X", getMemFree()));
        entry->CanBeSelected(false);
        menu += entry;

        ctrpfHeap = new MenuEntry(Utils::Format("Ctrpf MemFree: %08X", Heap::SpaceFree()));
        ctrpfHeap->CanBeSelected(false);
        menu += ctrpfHeap;

        menu += new MenuEntry("TB 1", nullptr, [](MenuEntry *entry)
        {
            MessageBox("Title", "1 line text")();
        });

        menu += new MenuEntry("TB 2", nullptr, [](MenuEntry *entry)
        {
            MessageBox("Title", "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?")();
        });

        menu += new MenuEntry("TB 3", nullptr, [](MenuEntry *entry)
        {
            MessageBox("1 line text")();
        });

        /*menu += new MenuEntry("Notification R", []
        (MenuEntry *entry)
        {
            if (Controller::IsKeyDown(R))
                OSD::Notify("Woooow");
        });*/

#endif

        menu.SyncronizeWithFrame(true);
        // Launch menu and mainloop
        menu.Run();

        // Exit plugin
        return (0);
    }
}

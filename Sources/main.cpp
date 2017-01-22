#include "CTRPluginFramework.hpp"
#include "3DS.h"

namespace CTRPluginFramework
{
    // This function is called on the plugin starts, before main
    void    PatchProcess(void)
    {
        u64 tid = Process::GetTitleID();

        // Patch game to prevent deconnection from Stream / debugger
        if (tid == 0x0004000000175E00 
            || tid == 0x0004000000164800)
        {
            u32     patch  = 0xE3A01000;
            Process::Patch(0x003DFFD0, (u8 *)&patch, 4);
        }
    }

    int    main(void)
    {   

        Menu    menu("Home");

        MenuFolder *folder = new MenuFolder("Folder #1");

        for (int i = 0; i < 10; i++)
        {
            char buf[40];
            sprintf(buf, "Entry #%d", i);
            menu.Append(new MenuEntry(buf));
        }

        for (int i = 0; i < 10; i++)
        {
            char buf[40];
            sprintf(buf, "Folder Entry #%d", i);
            folder->Append(new MenuEntry(buf));
        }

        menu.Append(folder);
        menu.Append(folder);
        menu.Append(folder);
        menu.Run();
        // Exit plugin
        return (0);


    }
}


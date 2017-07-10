#include "CTRPluginFramework.hpp"
#include "cheats.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include <cstdio>
#include <cctype>
#include <vector>
#include "CTRPluginFrameworkImpl/arm11kCommands.h"
#include "3DS.h"
#include "CTRPluginFrameworkImpl/Menu/HotkeysModifier.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "ctrulib/allocator/linear.h"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryTools.hpp"

namespace CTRPluginFramework
{    
    // This function is called on the plugin starts, before main
    void    PatchProcess(void)
    {
        u64 tid = Process::GetTitleID();

        // Pokemon Moon / Sun
        // Patch game to prevent deconnection from Stream / debugger
        /*if (tid == 0x0004000000175E00 
            || tid == 0x0004000000164800)
        {
            u32     patch  = 0xE3A01000;
            // Patch
            Process::Patch(0x003DFFD0, reinterpret_cast<u8 *>(&patch), 4); 
        }*/
    }

    static u32  g_encAboutSize = 153;
    static u32  g_encAbout[153] =
    {
        0x00000028, 0x00000154, 0x000001A8, 0x000001A8,
        0x000001DC, 0x00000094, 0x000001D8, 0x000001AC,
        0x000001F4, 0x000001B8, 0x0000018C, 0x00000194,
        0x000000B0, 0x00000194, 0x000001BC, 0x000001F0,
        0x00000080, 0x0000018C, 0x0000019C, 0x00000198,
        0x000001A8, 0x00000094, 0x000001AC, 0x00000198,
        0x000001B0, 0x000001B0, 0x000000A8, 0x000001A4,
        0x000001D4, 0x0000001C, 0x0000010C, 0x000001A8,
        0x0000019C, 0x00000180, 0x000000BC, 0x00000138,
        0x00000184, 0x000001C8, 0x000000A0, 0x00000034,
        0x00000008, 0x00000110, 0x00000194, 0x000001E4,
        0x000001A4, 0x000000B4, 0x0000019C, 0x00000184,
        0x00000198, 0x000001B8, 0x000001C0, 0x000001B8,
        0x00000194, 0x000001C4, 0x000001BC, 0x000001A0,
        0x00000198, 0x000000A4, 0x000001AC, 0x00000194,
        0x000001A0, 0x000000B4, 0x000001EC, 0x000001FC,
        0x00000190, 0x00000180, 0x000001D8, 0x00000198,
        0x000001DC, 0x00000094, 0x000001C8, 0x000001A0,
        0x000000C8, 0x0000000C, 0x00000188, 0x000001FC,
        0x000001E0, 0x000001F4, 0x000001F4, 0x000000D4,
        0x000000BC, 0x000000B8, 0x00000194, 0x000001A8,
        0x000001C0, 0x000001B4, 0x000001CC, 0x00000194,
        0x00000098, 0x000001A8, 0x00000194, 0x00000198,
        0x0000008C, 0x00000100, 0x000001AC, 0x000001A0,
        0x00000184, 0x000000B0, 0x0000013C, 0x00000198,
        0x000001CC, 0x000000A8, 0x00000114, 0x0000014C,
        0x00000168, 0x00000164, 0x00000130, 0x00000098,
        0x00000170, 0x00000184, 0x000001EC, 0x000001A0,
        0x000001A4, 0x000001BC, 0x000001C4, 0x00000024,
        0x00000038, 0x0000010C, 0x0000018C, 0x00000188,
        0x00000190, 0x000000A4, 0x000001B0, 0x000001E4,
        0x000001A4, 0x000001A0, 0x000000B8, 0x000001EC,
        0x000001BC, 0x00000084, 0x000001C0, 0x00000198,
        0x000001D0, 0x000001A8, 0x000001D0, 0x000001CC,
        0x000000A0, 0x000001A0, 0x00000190, 0x000001C8,
        0x000000B0, 0x00000190, 0x000001F4, 0x000001F0,
        0x000001D4, 0x00000190, 0x000001C4, 0x0000008C,
        0x000001B0, 0x00000180, 0x000001D0, 0x00000188,
        0x00000098,
    };


    // Function to pass to plugin to decode the About text
    void    Decoder(std::string &output, void *input)
    {
        u32     size = g_encAboutSize;
        u32     *in = static_cast<u32 *>(input);
        int     i = 0;

        while (size)
        {
            u32 c = *in++;

            c = (c >> 2) ^ (i++ & 0xF);

            output.push_back(static_cast<char>(c));

            size--;
        }
    }

    //  
    std::string about = u8"\n" \
        u8"This plugin has been made by\n" \
        u8"Mega-Mew.\n\n" \
        u8"More information and updates to:\n" \
        u8"https://github.com/Mega-Mew/CTRPF-Plugins\n" \
        u8"Feel free to report any issues here.";

    /*
    // Function to pass to plugin to decode the About text
    void    Decoder(std::string &output, void *input)
    {
        u32     size = g_encAboutSize;
        u32     *in = static_cast<u32 *>(input);
        int     i = 0;

        while (size)
        {
            u32 c = *in++;

            c = (c >> 2) ^ (i++ & 0xF);

            output += static_cast<char>(c);

            size--;
        }
    }*/
    
#define FORMAT(str, fmt, ...) {char buffer[0X100] = {0}; sprintf(buffer, fmt, ##__VA_ARGS__); str += buffer;}
    
    void    GetHandlesInfo(MenuEntry *entry)
    {
        std::string &note = entry->Note();
        note.clear();

        KProcessHandleTable table;
        std::vector<HandleDescriptor>  handles;

        ProcessImpl::GetHandleTable(table, handles);

        // Display table infos
        FORMAT(note, "Handle table: %08X\n", (u32)table.handleTable);
        FORMAT(note, "Max handles: %d\n", table.maxHandle);
        FORMAT(note, "Handles open: %d\n", table.openedHandleCounter);
        FORMAT(note, "Next HandleDescriptor: %08X\n", (u32)table.nextOpenHandleDecriptor);
        FORMAT(note, "Total handles: %d\n", table.totalHandles);
        FORMAT(note, "Handles in use: %d\n\n", table.handlesCount);

        for (HandleDescriptor &desc : handles)
        {
            u32 pid = arm11kGetKProcessId(desc.kObjectPointer);
            FORMAT(note, "Handle: %08X, KObj: %08X, Pid: %d\n", desc.handleInfo, desc.kObjectPointer, pid);
        }

        MessageBox("Done !")();
    }

    u32 spaceFree1;
    u32 spaceFree2;
    u32 spaceFree3;
    void func(void)
    {
        std::string str;
        Keyboard kb;

        kb.Open(str);

        spaceFree2 = getMemFree();
    }

    void func2(void)
    {
        spaceFree1 = getMemFree();
        func();
        spaceFree3 = getMemFree();
    }

    int    main(void)
    {
        PluginMenu      *m = new PluginMenu("Zelda Ocarina Of Time 3D", about);// g_encAbout, Decoder);
        PluginMenu      &menu = *m;
       

        menu.SetHexEditorState(false);
        /*
        ** Tests
        ********************/

        MenuEntry *entry = new MenuEntry("Heap infos", nullptr,
            [](MenuEntry *entry)
            {
            char buff[0x200] = { 0 };
            std::string str("This is a test");
            sprintf(buff, "Heap used: %08X\nHeap free: %08X\nLinear free: %08X\nsizeof(std::string) = %08X\n%08X", getMemUsed(), getMemFree(), linearSpaceFree(), sizeof(std::string), sizeof(str));

            (MessageBox(buff))();
          //  func2();

           // sprintf(buff, "1: %08X \n2: %08X\n3: %08X\nMenuEntryImpl: %08X,\n MenuEntryCheat: %08X", spaceFree1, spaceFree2, spaceFree3, sizeof(MenuEntryImpl), sizeof(MenuEntryTools));
           // (MessageBox(buff))();
        });

        entry->SetArg(m);
        m->Append(entry);

        /*
        ** Movements codes
        ********************/

        MenuFolder *folder = new MenuFolder("Movement");

        folder->Append(new MenuEntry("MoonJump (\uE000)", MoonJump, "Press \uE000 to be free of the gravity."));
        folder->Append(new MenuEntry("Fast Move (\uE077 + \uE054)", MoveFast, "Use \uE077 while pressing \uE054 to move very fast. Be careful of the loading zone, it might put you out of bound."));
        folder->Append(new MenuEntry("Epona have max carrots", EponaMaxCarrots));
        folder->Append(new MenuEntry("Epona have max carrots", EponaInfiniteCarrotsAllAreas));
        folder->Append(new MenuEntry("Epona MoonJump (\uE054 + \uE002)", EponaMoonJump));

        menu.Append(folder);

        /*
        ** Battle codes
        ******************/

        folder = new MenuFolder("Battle", "Need some boosters for your fights ?");
        folder->Append(new MenuEntry("Refill Heart (\uE058)", RefillHeart, "Running low on heart ?\nThen touch the screen to fill you in."));
        folder->Append(new MenuEntry("Refill Magic (\uE058)", RefillLargeMagicbar, "Running low on magic ?\nThen touch the screen to refill the magic bar."));
        folder->Append(new MenuEntry("Unlock Heart", MaxHeart));
        folder->Append(new MenuEntry("Unlock Magic", UnlockMagic));
        folder->Append(new MenuEntry("Unlock Large Magic", UnlockLargeMb));
        folder->Append(new MenuEntry("Spin Attack", SpinAttack));
        folder->Append(new MenuEntry("Sword Glitch", SwordGlitch));
        folder->Append(new MenuEntry("Sticks are in fire", StickFire));

        menu.Append(folder);

        /*
        ** Inventory codes
        *******************/

        folder = new MenuFolder("Inventory");

        MenuFolder *sword = new MenuFolder("Swords");
        sword->Append(new MenuEntry("Unlock Kokiri Sword", UnlockKokiriSword));
        sword->Append(new MenuEntry("Unlock Excalibur Sword", UnlockExcaliburSword));
        sword->Append(new MenuEntry("Unlock Biggoron Sword", UnlockBiggoronSword));
        sword->Append(new MenuEntry("Unlock All Swords", UnlockAllSwords));

        MenuFolder *shield = new MenuFolder("Shield");
        shield->Append(new MenuEntry("Unlock Wood Shield", UnlockWoodShield));
        shield->Append(new MenuEntry("Unlock Hyrule Shield", UnlockHyruleShield));
        shield->Append(new MenuEntry("Unlock Mirror Shield", UnlockMirrorShield));
        shield->Append(new MenuEntry("Unlock All Shields", UnlockAllShields));

        MenuFolder *suits = new MenuFolder("Suits");
        suits->Append(new MenuEntry("Unlock Kokiri Suits", UnlockKokiriSuits));
        suits->Append(new MenuEntry("Unlock Zora Suits", UnlockZoraSuits));
        suits->Append(new MenuEntry("Unlock Goron Suits", UnlockGoronSuits));
        suits->Append(new MenuEntry("Unlock All Suits", UnlockAllSuits));

        MenuFolder *items = new MenuFolder("Items");
        items->Append(new MenuEntry("Infinite Arrows", InfiniteArrows));
        items->Append(new MenuEntry("Infinite Nuts", InfiniteNuts));
        items->Append(new MenuEntry("Infinite Sticks", InfiniteStick));
        items->Append(new MenuEntry("Infinite Bomb", InfiniteBomb));
        items->Append(new MenuEntry("Infinite Bombchu", InfiniteBombchu));
        items->Append(new MenuEntry("Infinite Slingshot", InfiniteSlingshot));

        folder->Append(new MenuEntry("100 Skulltulas", Skulltulas));
        folder->Append(new MenuEntry("Max Rupees", MaxRupees));


        folder->Append(sword);
        folder->Append(shield);
        folder->Append(suits);
        folder->Append(items);

        menu.Append(folder);

        /*
        Time codes
        ***********/
        folder = new MenuFolder("Time");

        folder->Append(new MenuEntry("Sunrise (\uE053 + \uE07B)", Sunrise));
        folder->Append(new MenuEntry("Daytime (\uE053 + \uE07D)", Daytime));
        folder->Append(new MenuEntry("Sunset (\uE053 + \uE07C)", Sunset));
        folder->Append(new MenuEntry("Night (\uE053 + \uE07A)", Night));
        folder->Append(new MenuEntry("Sky is always raining", AlwaysRaining));
        menu.Append(folder);

        /*
        ** Quest
        ************/

        folder = new MenuFolder("Quest");

        folder->Append(new MenuEntry("Unlock All Best Equipment", UnlockAllBest));
        folder->Append(new MenuEntry("Unlock All Stones", UnlockAllStones));
        folder->Append(new MenuEntry("Unlock All Medallions", UnlockAllMedallions));
        folder->Append(new MenuEntry("Unlock Enhanced Defense", UnlockEnhancedDefense));
        folder->Append(new MenuEntry("Unlock Heart Pieces", UnlockHeartpieces));
        folder->Append(new MenuEntry("Infinite Small Dungeon Keys", InfiniteSmallKeysAllDungeons));
        folder->Append(new MenuEntry("Unlock Map and Boss Key of all Dungeons", HaveMapCompassAndBossKeyAllDungeon));
        menu.Append(folder);
        /*
        ** Misc codes
        *************/
        folder = new MenuFolder("Misc.");

        folder->Append(new MenuEntry("Giant Link", GiantLink));
        folder->Append(new MenuEntry("Normal Link", NormalLink));
        folder->Append(new MenuEntry("Mini Link", MiniLink));
        folder->Append(new MenuEntry("Paper Link", PaperLink));
        folder->Append(new MenuEntry("Link always have his child voice", AlwaysChildLinkVoice));
        folder->Append(new MenuEntry("Link always have his adult voice", AlwaysAdultLinkVoice));
        folder->Append(new MenuEntry("Open Chest Many Times", OpenAnyChestInTheGameAsManyTimes));
        folder->Append(new MenuEntry("Collect Heart Piece Many Times", CollectHeartPiecesInOverworldAsMany));
        folder->Append(new MenuEntry("No Damage From Falling", NeverTakeDamageFromFalling));
        folder->Append(new MenuEntry("Giant knife won't break", GiantsKnifeNeverBreaks));

        menu.Append(folder);

        menu.Callback([] {Sleep(Milliseconds(1)); });
        // Launch menu and mainloop
        menu.Run();

        // Exit plugin
        return (0);
    }
}

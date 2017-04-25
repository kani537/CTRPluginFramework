#include "CTRPluginFramework.hpp"
#include "cheats.hpp"
#include "ctrulib/util/utf.h"
#include "CTRPluginFrameworkImpl/Graphics/MessageBoxImpl.hpp"

namespace CTRPluginFramework
{    
    // This function is called on the plugin starts, before main
    void    PatchProcess(void)
    {
        u64 tid = Process::GetTitleID();

        // Pokemon Moon / Sun
        // Patch game to prevent deconnection from Stream / debugger
        if (tid == 0x0004000000175E00 
            || tid == 0x0004000000164800)
        {
            u32     patch  = 0xE3A01000;
            u32     original = 0;
            // Patch and get the original data
            Process::Patch(0x003DFFD0, (u8 *)&patch, 4); 
        }
    }

    void    TestStringKeyboard(MenuEntry *entry)
    {
        Keyboard keyboard("Enter any text");

        std::string str;

        if (keyboard.Open(str) != -1)
        {
            entry->Name() = str;
        }
    }

    void    TestMsgBox1(MenuEntry *entry)
    {
        MessageBoxImpl msgBox("This is a message !");

        msgBox();
    }

    void    TestMsgBox2(MenuEntry *entry)
    {
        if (Controller::IsKeysPressed(A + L))
        {
            MessageBoxImpl msgBox("Do you want to rename it ?", DialogType::DialogYesNo);

            // Display MessageBox and check user choice
            if (msgBox())
            {
                // Display keyboard etc...
            }
        }
    }

    int    main(void)
    {
        PluginMenu      *m = new PluginMenu("Zelda Ocarina Of Time 3D");
        PluginMenu      &menu = *m;

        std::string note = "t: ";

        char buf[10] = { 0 };
        encode_utf8((u8 *)buf, 0xE000);

        note += buf;

        MenuEntry *entry = new MenuEntry("Test Keyboard", note);

        entry->SetMenuFunc(TestStringKeyboard);
        menu.Append(entry);

        entry = new MenuEntry("Test Box 1");

        entry->SetMenuFunc(TestMsgBox1);
        menu.Append(entry);

        entry = new MenuEntry("Test Box 2", TestMsgBox2);

        menu.Append(entry);

    
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

        // Launch menu and mainloop
        menu.Run();

        // Exit plugin
        return (0);
    }
}

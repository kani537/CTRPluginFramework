#include "CTRPluginFramework.hpp"

#include "CTRPluginFramework.hpp"
#include "ctrulib/services/hid.h"
#include "cheats.hpp"
#include <cstdio>

#include <string>
#include <vector>

#include "ctrulib/services/gspgpu.h"
#include "3DS.h"

namespace CTRPluginFramework
{
    extern "C" vu32* hidSharedMem;
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

    void    ZLCamera(MenuEntry *entry)
    {
        if (Controller::IsKeyDown(Key::Touchpad))
        {
            char buffer[60] = {0};
            u32 Id = (u32)hidSharedMem[42 + 4];

            u64 t = *((u64*)&hidSharedMem[42]);
            u64 t1 = *((u64 *)&hidSharedMem[44]);

            sprintf(buffer, "%016llX %016llX %d \n", t, t1, Id);
            if (entry->Note().size() > 500)
                entry->Note().clear();
            entry->Note() += buffer;
        }
        if (Controller::IsKeyDown(Key::ZL))
            Controller::InjectTouch(10, 20);
    }

    void    ZLFirstButton(MenuEntry *entry)
    {
        if (Controller::IsKeyDown(Key::ZL))
            Controller::InjectTouch(300, 20);
    }

    void    ZRSecondButton(MenuEntry *entry)
    {
        if (Controller::IsKeyDown(Key::ZR))
            Controller::InjectTouch(300, 210);
    }

    void    CStickToDPAD(MenuEntry *entry)
    {
        if (Controller::IsKeyDown(Key::CStick))
        {
            u32 keys = (hidKeysHeld() & Key::CStick) >> 20;
            Controller::InjectKey(keys);
        }
    }

    void    ZLToL(MenuEntry *entry)
    {
        if (Controller::IsKeyDown(Key::ZL))
        {
            //u32 keys = (hidKeysHeld() & Key::L);
            Controller::InjectKey(Key::L);
        }  
    }

    void    ZRToR(MenuEntry *entry)
    {
        if (Controller::IsKeyDown(Key::ZR))
        {
            //u32 keys = (hidKeysHeld() & Key::R);
            Controller::InjectKey(Key::R);
        }  
    }

    void    Overlay(MenuEntry *entry)
    {
        u64 tick = svcGetSystemTick();
        u8  c = (tick >> 2) & 0b111;

        Color black = Color();//(tick >> 8) & 0xFF, (tick >> 16) & 0xFF, (tick) & 0xFF);
        Color blank = Color(255, 255, 255);//(tick >> 16) & 0xFF, (tick) & 0xFF, (tick >> 8) & 0xFF);
        
        if (Controller::IsKeyPressed(Key::X))
        {
            switch (c)
            {
                case 1:
                    black = Color();
                    blank = Color(255, 255, 255);
                    OSD::Notify("Nanquitas says Hello !", blank, black);
                    break;
                case 2:
                    black = Color(0, 255, 255, 200);
                    OSD::Notify("I wanna eat an hot dog !", blank, black);
                    break;
                case 3:
                    black = Color(0, 99, 0, 150);
                    OSD::Notify("I'm a notification", blank, black);
                    break;
                case 4:
                    OSD::Notify("Whaaaaaaaaaaaaaaaaaaat !?!", blank, black);
                    break;
                case 5:
                    black = Color(0, 66, 0xCC, 150);
                    OSD::Notify("I love chocolate !", blank, black);
                    break;
                default:
                    black = Color(255, 0, 0, 150);
                    OSD::Notify("I'm an important notification", blank, black);
                    break;                    
            }            
        }
    }

    void    TouchCursor(MenuEntry *entry)
    {
            // Draw Touch Cursor
          /*  if (Touch::IsDown())
            {
                UIntVector t(Touch::GetPosition());
                int posX = t.x - 2;
                int posY = t.y - 1;

                Renderer::SetTarget(BOTTOM);
                Icon::DrawHandCursor(posX, posY);
                Screen::Bottom->Flush();
            }*/
    }

    int    main(void)
    {   
        PluginMenu  *m = new PluginMenu("Zelda Ocarina Of Time 3D");
        PluginMenu    &menu = *m;       
        
        int res4 = 0;
        int res3 = 0;
        File    file;
        //char    buffer[0x100] = {0};

        if ((File::Open(file, "Test.txt")) == 0)
        {
            file.WriteLine("Yeah !! This is a line written by WriteLine.");
            file.WriteLine("Yeah !! This is a second line written by WriteLine.");
            //u64 size = file.GetSize();
            //file.Rewind();
           // file.Read(buffer, size);
            //file.Close();
        }
            // Enable New3DS CPU Frequencies
        osSetSpeedupEnable(true);
        std::string ls = "Files in the current working directory: \n";        
        std::vector<std::string> lsv;

        Directory base;
        Directory::Open(base, "", false); //Open current working directory (plugin/<currentTitleID>/)
        base.ListFiles(lsv); // listing all files in base (not directory)
        // An other version for the example
        //base.ListFiles(lsv, ".txt"); // listing all files in base (not directory) which contains .txt in their name
        
        // Adding all the names in the list into ls string
        for (int i = 0; i < lsv.size(); i++)
        {
            ls += lsv[i] + "\n";
        }
        // this add the content of the file we've read earlier in the ls string
        //ls += buffer;


        /*
        ** Movements codes
        ********************/

        MenuFolder *folder = new MenuFolder("Movement", ls);

        folder->Append(new MenuEntry("MoonJump (\uE000)", MoonJump, "Press \uE000 to be free of the gravity."));
        folder->Append(new MenuEntry("Fast Move (\uE077 + \uE054)", MoveFast, "Use \uE077 while pressing \uE054 to move very fast. Be careful of the loading zone, it might put you out of bound."));
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


        menu.Append(folder);

        /*
        ** Misc codes
        *************/
        folder = new MenuFolder("Misc.");

        folder->Append(new MenuEntry("Unlock All Best Equipment", UnlockAllBest));
        folder->Append(new MenuEntry("Unlock All Medallions", UnlockAllMedallions));
        folder->Append(new MenuEntry("Giant Link", GiantLink));
        folder->Append(new MenuEntry("Normal Link", NormalLink));
        folder->Append(new MenuEntry("Mini Link", MiniLink));
        folder->Append(new MenuEntry("Paper Link", PaperLink));
        folder->Append(new MenuEntry("Open Chest Many Times", OpenAnyChestInTheGameAsManyTimes));
        folder->Append(new MenuEntry("Collect Heart Piece Many Times", CollectHeartPiecesInOverworldAsMany));
        folder->Append(new MenuEntry("No Damage From Falling", NeverTakeDamageFromFalling));

        menu.Append(folder);
        menu.Append(new MenuEntry("\uE002 to send a notification", Overlay));
        menu.Append(new MenuEntry("\uE054 = Camera button", ZLCamera));
        menu.Append(new MenuEntry("\uE054 = First object button", ZLFirstButton));
        menu.Append(new MenuEntry("\uE055 = Second Object button", ZRSecondButton));
        menu.Append(new MenuEntry("CStick as \uE041", CStickToDPAD));
        menu.Append(new MenuEntry("\uE054 as \uE004", ZLToL));
        menu.Append(new MenuEntry("\uE055 as \uE005", ZRToR));
        menu.Append(new MenuEntry ("This is an incredibly long entry. Stay here a little to make it scroll and see the entire text. \uE000 \uE001 \uE002 \uE003 \uE004 \uE005 \uE006 \uE040 \uE041 \uE042 \uE043 \uE044 \uE045"));
        menu.Append(new MenuEntry("Display touch cursor", TouchCursor));
        file.Close();
        // Launch menu and mainloop
        menu.Run();

        // Exit plugin
        return (0);
    }
}

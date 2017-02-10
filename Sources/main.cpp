#include "CTRPluginFramework.hpp"

#include "CTRPluginFramework/Directory.hpp"
#include "CTRPluginFramework/File.hpp"
#include "ctrulib/services/hid.h"
#include "cheats.hpp"
#include <cstdio>

#include <string>
#include <vector>

#include "ctrulib/services/gspgpu.h"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
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
            if (entry->note.size() > 500)
                entry->note.clear();
            entry->note += buffer;
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
            if (Touch::IsDown())
            {
                UIntVector t(Touch::GetPosition());
                int posX = t.x - 2;
                int posY = t.y - 1;

                Renderer::SetTarget(BOTTOM);
                Icon::DrawHandCursor(posX, posY);
                Screen::Bottom->Flush();
            }
    }

    u32     findNearestSTMFD(u32 base, u32 pos) 
    {
        if (pos < base)
        {
            return 0;
        }
        pos = pos - pos % 4;
        u32 term = pos - 0x1000;
        if (term < base)
        {
            term = base;
        }
        while (pos >= term) {
            if (*(u16*)(pos + 2) == 0xe92d){
                return pos;
            }
            pos -= 4;
        }
        return 0;
    }

u32     searchBytes(u32 startAddr, u32 endAddr, u8* pat, int patlen, int step)
{
    u32 lastPage = 0;
    u32 pat0 = ((u32*)pat)[0];

    while (1)
    {
        if (startAddr + patlen >= endAddr)
        {
                return 0;
        }
        if (*((u32*)(startAddr)) == pat0)
        {
            if (memcmp((u32*) startAddr, pat, patlen) == 0)
            {
                return startAddr;
            }
        }
        startAddr += step;
    }
    return 0;
}

    u32     locateSwapBuffer(u32 startAddr, u32 endAddr) 
    {
        
        static u32 pat[] = { 0xe1833000, 0xe2044cff, 0xe3c33cff, 0xe1833004, 0xe1824f93 };
        static u32 pat2[] = { 0xe8830e60, 0xee078f9a, 0xe3a03001, 0xe7902104 };
        static u32 pat3[] = { 0xee076f9a, 0xe3a02001, 0xe7901104, 0xe1911f9f, 0xe3c110ff};

        u32 addr = searchBytes(startAddr, endAddr, (u8 *)pat, sizeof(pat), 4);
        if (!addr) 
        {
            addr = searchBytes(startAddr, endAddr, (u8 *)pat2, sizeof(pat2), 4);
        }
        if (!addr) 
        {
            addr = searchBytes(startAddr, endAddr, (u8 *)pat3, sizeof(pat3), 4);
        }
        return (findNearestSTMFD(startAddr, addr));
    }

    int    main(void)
    {   
        PluginMenu  *m = new PluginMenu("Zelda Ocarina Of Time 3D");
        PluginMenu    &menu = *m;

        /*    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus id semper ligula. Vivamus sollicitudin lacinia ligula, vel hendrerit massa posuere in. Aliquam eget euismod tortor, vel ultricies sem. Donec tempor odio vel neque suscipit finibus ac at quam. Cras pharetra, massa scelerisque rutrum pretium, sapien tortor lobortis elit, vel euismod dui dolor non leo. Praesent sodales sagittis purus quis feugiat. Praesent est massa, gravida et ultrices in, aliquet ac tortor. Nam id tempus dolor. Pellentesque lobortis porta sagittis. Phasellus malesuada pulvinar porttitor.\n\n" \
"Donec ultrices tortor sit amet nibh tincidunt dapibus. Sed interdum dignissim nibh, vel convallis velit volutpat ut. Phasellus blandit, ligula ut varius luctus, quam mi maximus massa, at mattis sem ligula ac mi. Vestibulum tempor fermentum pretium. Phasellus in metus elit. Donec eget massa eu libero lacinia accumsan. Phasellus non velit eget mauris gravida ornare. Sed sodales nulla in dictum pellentesque.\n\n" \
"Nam magna nunc, posuere ut nisi id, cursus ultricies dui. Suspendisse efficitur id dolor sit amet rhoncus. Sed tincidunt arcu nunc, quis maximus nunc maximus a. Quisque pellentesque libero quis urna rhoncus, eget rutrum diam tincidunt. Nulla eget ultricies nisl, in condimentum nulla. Nullam nec turpis a tellus faucibus dictum ut quis eros. Etiam placerat fringilla nisl at convallis. Ut sit amet ligula accumsan, feugiat neque maximus, sollicitudin nulla. Curabitur posuere, enim ut placerat tincidunt, dui tellus ullamcorper lorem, eu feugiat dui leo quis velit. Morbi non velit eget ipsum sodales sagittis in vel urna.\n\n" \
"unc sem eros, fermentum sed justo sit amet, condimentum tempus nunc. Mauris elementum vulputate tempus. Proin iaculis justo rhoncus, lobortis massa nec, tempus dolor. Donec et dictum erat. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Sed convallis arcu et nisi posuere, vel fermentum est cursus. Phasellus hendrerit, tellus id bibendum auctor, odio elit blandit justo, sed porttitor diam lorem eu lacus. Phasellus ac nibh venenatis, sagittis enim ultrices, efficitur tellus. Vestibulum ullamcorper egestas nunc eget volutpat. In tempus eros eu ipsum feugiat placerat. Cras iaculis odio nec nunc faucibus, in blandit turpis lacinia.\n\n" \
"Aenean a massa lacinia, elementum urna aliquam, sodales lorem. Vivamus hendrerit egestas orci vitae interdum. Aenean ultricies justo mi, et porta lacus placerat at. Sed faucibus nulla viverra dolor luctus, eget blandit sapien vestibulum. Duis commodo varius vulputate. Quisque ut dui lorem. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque quis tellus porttitor, pellentesque sem mollis, eleifend felis.\n\n";
        */
        //char temp[100] = {0};
        
        MemInfo minfo;
        PageInfo pinfo;

        svcQueryMemory(&minfo, &pinfo, 0x100000);

        u32   addr = locateSwapBuffer(minfo.base_addr, minfo.base_addr + minfo.size);
        
        
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

        char buffer[100];

        sprintf(buffer, "outaddr: %08X", addr);
        ls += buffer;
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

#include "CTRPluginFramework.hpp"
#include "cheats.hpp"

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

    int    main(void)
    {   

        Menu    menu("Zelda Ocarina Of Time 3D");

        std::string text = 
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus id semper ligula. Vivamus sollicitudin lacinia ligula, vel hendrerit massa posuere in. Aliquam eget euismod tortor, vel ultricies sem. Donec tempor odio vel neque suscipit finibus ac at quam. Cras pharetra, massa scelerisque rutrum pretium, sapien tortor lobortis elit, vel euismod dui dolor non leo. Praesent sodales sagittis purus quis feugiat. Praesent est massa, gravida et ultrices in, aliquet ac tortor. Nam id tempus dolor. Pellentesque lobortis porta sagittis. Phasellus malesuada pulvinar porttitor.\n\n" \
"Donec ultrices tortor sit amet nibh tincidunt dapibus. Sed interdum dignissim nibh, vel convallis velit volutpat ut. Phasellus blandit, ligula ut varius luctus, quam mi maximus massa, at mattis sem ligula ac mi. Vestibulum tempor fermentum pretium. Phasellus in metus elit. Donec eget massa eu libero lacinia accumsan. Phasellus non velit eget mauris gravida ornare. Sed sodales nulla in dictum pellentesque.\n\n" \
"Nam magna nunc, posuere ut nisi id, cursus ultricies dui. Suspendisse efficitur id dolor sit amet rhoncus. Sed tincidunt arcu nunc, quis maximus nunc maximus a. Quisque pellentesque libero quis urna rhoncus, eget rutrum diam tincidunt. Nulla eget ultricies nisl, in condimentum nulla. Nullam nec turpis a tellus faucibus dictum ut quis eros. Etiam placerat fringilla nisl at convallis. Ut sit amet ligula accumsan, feugiat neque maximus, sollicitudin nulla. Curabitur posuere, enim ut placerat tincidunt, dui tellus ullamcorper lorem, eu feugiat dui leo quis velit. Morbi non velit eget ipsum sodales sagittis in vel urna.\n\n" \
"unc sem eros, fermentum sed justo sit amet, condimentum tempus nunc. Mauris elementum vulputate tempus. Proin iaculis justo rhoncus, lobortis massa nec, tempus dolor. Donec et dictum erat. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Sed convallis arcu et nisi posuere, vel fermentum est cursus. Phasellus hendrerit, tellus id bibendum auctor, odio elit blandit justo, sed porttitor diam lorem eu lacus. Phasellus ac nibh venenatis, sagittis enim ultrices, efficitur tellus. Vestibulum ullamcorper egestas nunc eget volutpat. In tempus eros eu ipsum feugiat placerat. Cras iaculis odio nec nunc faucibus, in blandit turpis lacinia.\n\n" \
"Aenean a massa lacinia, elementum urna aliquam, sodales lorem. Vivamus hendrerit egestas orci vitae interdum. Aenean ultricies justo mi, et porta lacus placerat at. Sed faucibus nulla viverra dolor luctus, eget blandit sapien vestibulum. Duis commodo varius vulputate. Quisque ut dui lorem. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque quis tellus porttitor, pellentesque sem mollis, eleifend felis.\n\n";

        /*
        ** Movements codes
        ********************/

        MenuFolder *folder = new MenuFolder("Movement", text);

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
        
        // Launch menu and mainloop
        menu.Run();

        // Exit plugin
        return (0);
    }
}

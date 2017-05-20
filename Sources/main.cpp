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
            // Patch
            Process::Patch(0x003DFFD0, reinterpret_cast<u8 *>(&patch), 4); 
        }
    }

    static u32  g_encAboutSize = 43;
    static u32  g_encAbout[43] =
    {
        0x00000028, 0x00000108, 0x0000019C, 0x0000008C,
        0x000001D0, 0x000001A4, 0x000001CC, 0x00000180,
        0x00000184, 0x0000019C, 0x000000A8, 0x000001F4,
        0x0000018C, 0x000001E0, 0x000001F4, 0x000000BC,
        0x00000194, 0x000001C8, 0x000001D8, 0x0000008C,
        0x000001D0, 0x000001DC, 0x000001A4, 0x000001DC,
        0x0000019C, 0x000001E8, 0xFFFFFF24, 0xFFFFFE88,
        0x000000B0, 0x000001F4, 0x000001BC, 0x000001F4,
        0x00000028, 0x0000013C, 0x0000018C, 0x000001B4,
        0x000001D4, 0x000001C0, 0x000001BC, 0x000001CC,
        0x000001A4, 0x000001E8, 0x00000090
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

            output += static_cast<char>(c);

            size--;
        }
    }

    void    TestStringKeyboard(MenuEntry *entry)
    {
        Keyboard keyboard("Enter any text");

        std::string str;

        // If user didn't abort the keyboard (B)
        if (keyboard.Open(str) != -1)
        {
            // Replace the entry's name by the user's text
            entry->Name() = str;
        }
    }

    void    TestMsgBox1(MenuEntry *entry)
    {
        MessageBox msgBox("This is a message !");

        msgBox();
    }

    void    TestMsgBox2(MenuEntry *entry)
    {
        if (Controller::IsKeysPressed(A + L))
        {
            MessageBox msgBox("Do you want to rename it ?", DialogType::DialogYesNo);

            // Display MessageBox and check user choice
            if (msgBox())
            {
                // Display keyboard etc...
            }
        }
    }

    // Pointer to the folder I'll hide / show
    MenuFolder  *g_folderToHide = nullptr;

    void    HideShow(MenuEntry *entry)
    {
        static bool isHidden = false;

        if (isHidden)
            g_folderToHide->Show();
        else
            g_folderToHide->Hide();

        isHidden = !isHidden;
    }

    void    Test(MenuEntry *entry)
    {
        Process::Patch(0x12345678, 0x12345678);
    }

    MenuEntry *g_e;

    void    Notif(MenuEntry *entry)
    {
        if (Controller::IsKeyPressed(X))
        {
            OSD::Notify("A new notification 2 !");
            g_e->Enable();
        }
            
    }

    void    Write(MenuEntry *entry)
    {
        OSD::WriteLine(0, "This is a test line...", 10, 10);

        OSD::WriteLine(1, "This is a test line...", 20, 10);
        OSD::WriteLine(1, "This is a test line...", 20, 30, 15);
    }

    // class pokemon
    VectorPokemon   *g_pokeList = nullptr;

    Pokemon::Pokemon(std::string name, u32 id) : ID(id), Name(name)
    {
    }

    void    Pokemon::InitList(void)
    {
        // Init our Vector if it doesn't exist yet, else clear it
        if (g_pokeList == nullptr)
            g_pokeList = new VectorPokemon;
        else
            g_pokeList->clear();

        // Create our list
        VectorPokemon &list = *g_pokeList;

        list.push_back(Pokemon("Bulbasaur", 1));
        list.push_back(Pokemon("Ivysaur", 2));
        list.push_back(Pokemon("Venusaur", 3));
        list.push_back(Pokemon("Charmander", 4));
        list.push_back(Pokemon("Charmeleon", 5));
        list.push_back(Pokemon("Squirtle", 6));
        list.push_back(Pokemon("Wartortle", 7));
        list.push_back(Pokemon("Blastoise", 8));
        list.push_back(Pokemon("Caterpie", 9));
        list.push_back(Pokemon("MetaPod", 10));
        list.push_back(Pokemon("Butterfree", 11));
        
        // Etc...
    }
    
    u32     Pokemon::GetList(VectorPokemon& output, std::string pattern)
    {
        VectorPokemon &list = *g_pokeList;

        // Clear list
        output.clear();

        // If pattern isn't specified return full list
        if (pattern.empty())
        {
            for (Pokemon &pokemon : list)
            {
                output.push_back(pokemon);
            }
        }

        // Else parse our list and found those matches the pattern
        else
        {
            for (Pokemon &pokemon : list)
            {
                if (pokemon.Name.find(pattern) != std::string::npos)
                    output.push_back(pokemon);
            }
        }

        // Return the count of found pokemon that matched the pattern
        return (output.size());
    }

    Pokemon     &Pokemon::GetPokemon(std::string name)
    {
        static Pokemon empty("Error pokemon", 0);

        for (Pokemon &pokemon : *g_pokeList)
            if (pokemon.Name.compare(name) == 0)
                return pokemon;

        return (empty);
    }

    Pokemon     &Pokemon::GetPokemon(u32 id)
    {
        static Pokemon empty("Error pokemon", 0);

        for (Pokemon &pokemon : *g_pokeList)
            if (pokemon.ID == id)
                return pokemon;

        return (empty);
    }

    void    PokemonOnInputChange(Keyboard &keyboard)
    {
        std::string     &input = keyboard.GetInput();
        VectorPokemon   list;

        // Get the list of Pokemon that matches the pattern entered by the user
        Pokemon::GetList(list, input);

        // If the list is empty, the user made an error:
        if (list.empty())
        {
            keyboard.SetError("No pokemon matches your input.\nCheck it and try again.");
            return;
        }

        // Check if the list that matches the pattern is small enough
        if (list.size() < 15)
        {
            // Create a keyboard with the list and tell the user to chose one
            std::vector<std::string>    pokeList;
            Keyboard                    listKeyboard("");

            for (Pokemon &pokemon : list)
                pokeList.push_back(pokemon.Name);

            // Populate the keyboard
            listKeyboard.Populate(pokeList);

            // I don't want the top screen to be displayed
            listKeyboard.DisplayTopScreen = false;

            // User can't abort the keyboard
            listKeyboard.CanAbort(false);

            // Get user choice
            int userChoice = listKeyboard.Open();

            // Set my main keyboard input
            input = pokeList[userChoice];

            // Close Qwerty keyboard
            keyboard.Close();
            return;
        }

        // The list is still too long, user must keep typing letters
        std::string error("Too much results, keep typing: " + list.size());
        keyboard.SetError(error);
    }

    void    PokemonKeyboard(MenuEntry *entry)
    {
        // Create QwertyKeyboard
        Keyboard        keyboard("Enter the pokemon name");
        std::string     output;

        // Function to call when user change input
        keyboard.OnInputChange(PokemonOnInputChange);

        // Open keyboard
        keyboard.Open(output);

        // Get Pokemon
        Pokemon &pokemon = Pokemon::GetPokemon(output);

        // Do something with the infos from the pokemon
        MessageBox("You've selected:\n" + pokemon.Name + "(" + std::to_string(pokemon.ID) + ")")();
    }

    int    main(void)
    {

        PluginMenu      *m = new PluginMenu("Zelda Ocarina Of Time 3D", g_encAbout, Decoder);
        PluginMenu      &menu = *m;
        
        Pokemon::InitList();

        /*
        ** Tests
        ********************/
        menu.Append(new MenuEntry("Pokemon Keyboard", nullptr, PokemonKeyboard));
        menu.Append(new MenuEntry("Notif X", Notif));
        g_e = new MenuEntry("Write", Write);
        menu.Append(g_e);
        // Set my global
        g_folderToHide = new MenuFolder("Folder to hide");
        // Append to the menu
        menu.Append(g_folderToHide);

        g_folderToHide->Append(new MenuEntry("Test Keyboard", nullptr, TestStringKeyboard));
        g_folderToHide->Append(new MenuEntry("Test Box 1", nullptr, TestMsgBox1));
        g_folderToHide->Append(new MenuEntry("Test Box 2", TestMsgBox2));
        g_folderToHide->Append(new MenuEntry("Hide", nullptr, HideShow));

        menu.Append(new MenuEntry("Hide", nullptr, HideShow));
        
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

#include "CTRPluginFramework.hpp"
#include "cheats.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include <cstdio>
#include <cctype>
#include <vector>
#include "CTRPluginFrameworkImpl/arm11kCommands.h"

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

    using StringVector = std::vector<std::string>;
    using StringIter = std::string::iterator;
    using StringConstIter = std::string::const_iterator;

    // Create our list of possibilities
    static const StringVector     g_possibilities =
    {
        "Bulbasaur",
        "Ivysaur",
        "Venusaur",
        "Charmander",
        "Charmeleon",
        "Charizard",
        "Squirtle",
        "Wartortle",
        "Blastoise",
        "Caterpie",
        "Metapod",
        "Butterfree",
        "Weedle",
        "Kakuna",
        "Beedrill",
        "Pidgey",
        "Pidgeotto",
        "Pidgeot",
        "Rattata",
        "Raticate",
        "Spearow",
        "Fearow",
        "Ekans",
        "Arbok",
        "Pikachu",
        "Raichu"
    };

    /**
    * @brief      Gets the Pokemons that matches the first letters of the input
    *
    * @param      output  The vector where to put the pokemons that matches the input
    * @param      input   The input string entered by the user
    *
    * @return     The amount of pokemons that matches the input.
    */
    int     GetMatches(StringVector &output, std::string &input)
    {
        // Clear the output
        output.clear();

        // Clone the input string but with forced lowcase
        std::string     lowcaseInput(input);

        for (char &c : lowcaseInput)
            c = std::tolower(c);

        // Parse our possibilities to find the matches
        for (const std::string &pokemon : g_possibilities)
        {
            StringIter      inputIt = lowcaseInput.begin();
            StringConstIter pokeIt = pokemon.begin();

            // Parse every letter of input while it matches the pokemon's name
            while (inputIt != lowcaseInput.end() && pokeIt != pokemon.end() && *inputIt == std::tolower(*pokeIt))
            {
                ++inputIt;
                ++pokeIt;
            }

            // If we're at the end of input then it matches the pokemon's name
            if (inputIt == lowcaseInput.end())
                output.push_back(pokemon);
        }

        return (output.size());
    }

    /**
    * @brief      This function will be called by the Keyboard everytime the input change
    *
    * @param      keyboard  The keyboard that called the function
    */
    void    OnInputChange(Keyboard &keyboard, InputChangeEvent &event)
    {
        if (event.type == InputChangeEvent::CharacterRemoved)
        {
            if (!keyboard.GetInput().size())
                keyboard.SetError("Not enough letters to do the search.\nKeep typing.");
            return;
        }            

        std::string  &input = keyboard.GetInput();

        // If input's length is inferior than 3, ask for more letters
        if (input.size() < 3)
        {
            keyboard.SetError("Not enough letters to do the search.\nKeep typing.");
            return;
        }

        // Else do the search
        StringVector    matches;


        // Search for matches
        int count = GetMatches(matches, input);

        // If we don't have any matches, tell the user
        if (!count)
        {
            keyboard.SetError("Nothing matches your input.\nTry something else.");
            return;
        }

        // If we have only one matches, complete the input
        if (count == 1)
        {
            input = matches[0];
            return;
        }

        // If we have more than 1, but less or equal than 10 matches, ask the user to choose which one
        if (count <= 10)
        {
            // Our new keyboard
            Keyboard    listKeyboard;

            // Populate the keyboard with the matches
            listKeyboard.Populate(matches);

            // User can't abort with B
            listKeyboard.CanAbort(false);

            // Nothing to display on the top screen
            listKeyboard.DisplayTopScreen = false;

            // Display the keyboard and get user choice
            int choice = listKeyboard.Open();

            // Complete the input
            input = matches[choice];
            return;
        }

        // We have too much results, the user must keep typing letters
        keyboard.SetError("Too many results: " + std::to_string(count) + "\nType more letters to narrow the results.");
    }

    /**
    * @brief      A cheat function that needs the user to select a pokemon before doing something
    *
    * @param      entry  The entry that called the function
    */
    void    SelectAPokemon(MenuEntry *entry)
    {
        // Store the selected pokemon's name
        std::string  output;

        // Create a keyboard
        Keyboard    keyboard("Which pokemon do you want ?");

        // Pass our OnInputChange function to the keyboard
        keyboard.OnInputChange(OnInputChange);

        // Open the keyboard
        if (keyboard.Open(output) != -1)
        {
            // Display the selected pokemon
            MessageBox("You have selected:\n" + output)();
        }
    }


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

    int    main(void)
    {

        PluginMenu      *m = new PluginMenu("Zelda Ocarina Of Time 3D", g_encAbout, Decoder);
        PluginMenu      &menu = *m;
       

        /*
        ** Tests
        ********************/
        //menu.Append(new MenuEntry("Pokemon Keyboard", nullptr, PokemonKeyboard));
        menu.Append(new MenuEntry("Get KProcess Handles", nullptr, GetHandlesInfo));
        menu.Append(new MenuEntry("Pokemon Keyboard", nullptr, SelectAPokemon));
        
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

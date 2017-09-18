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
#include "NTR.hpp"
#include "Hook.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"
#include <limits>
#include <random>
#include "CTRPluginFrameworkImpl/ActionReplay/ARCode.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARHandler.hpp"
#include <locale>
#include <cstring>
#include "csvc.h"
#include "NTRImpl.hpp"
#include "ctrulib/gpu/gx.h"

namespace CTRPluginFramework
{
    // This function is called on the plugin starts, before main
    void    PatchProcess(void)
    {

    }

    std::string about = u8"\n" \
        u8"Plugin for Zelda Ocarina Of Time, V3.0\n\n"
        u8"Most of these codes comes from Fort42 so a huge thanks to their original creator !!\n\n" \
        u8"GBATemp's release thread: goo.gl/Rz1uhj";
    
    void    Invincible(MenuEntry *entry)
    {
        static u32 original[4] = {0};

        if (entry->WasJustActivated())
        {
            if (original[0] == 0)
            {
                Process::Read32(0x0035D398, original[0]);
                Process::Read32(0x0035D3A8, original[1]);                
                Process::Read32(0x00352E24, original[2]);
                Process::Read32(0x00352E28, original[3]);              
            }

            Process::Write32(0x0035D398, 0xE3A00000);
            Process::Write32(0x0035D3A8, 0xEA000000);
            Process::Write32(0x00352E24, 0xE1D504B2);
            Process::Write32(0x00352E28, 0xE1A00000);
        }

        if (!entry->IsActivated())
        {
            if (original[0] && original[1])
            {
                Process::Write32(0x0035D398, original[0]);
                Process::Write32(0x0035D3A8, original[1]);
                Process::Write32(0x00352E24, original[2]);
                Process::Write32(0x00352E28, original[3]);
            }
        }
    }

    void    UnlockAllBottles(MenuEntry *entry)
    {
        Process::Write16(0x005879F6, 0x1414);
        Process::Write8(0x005879F8, 0x14);
        entry->Disable();
    }

    struct Item
    {
        u8  id;
        std::string name;
    };

    const std::vector<Item> g_bottleItems = 
    {
        {0x15, "Red potion"},
        {0x16, "Green potion"},
        {0x17, "Blue potion"},
        {0x18, "Fairy"},
        {0x19, "Fish"},
        {0x1A, "Milk, 2 Doses"},        
        {0x1F, "Milk, 1 dose"},
        {0x1B, "Letter"},
        {0x1C, "Blue flamme"},
        {0x1D, "Insect"},
        {0x1E, "Soul"},
        {0x20, "Spirit"},
        {0x14, "Empty" },
        {0xFF, "Locked" },
        {0x00, "Unknown / Error"}
    };

    class Bottle
    {
    public:

        Bottle(u32 id, u32 address) : ID(id), SelectedItem(0)
        {
            _address = reinterpret_cast<u8 *>(address);
        }

        ~Bottle(){}

        void    operator=(const Item &item)
        {
            SelectedItem = item.id;
            *_address = item.id;
        }

        void    WriteItem(void)
        {
            if (SelectedItem)
                *_address = SelectedItem;
        }

        const Item &GetCurrentItem(void)
        {
            u8 id = *_address;

            for (const Item &item : g_bottleItems)
                if (item.id == id)
                    return (item);

            return (g_bottleItems.back());
        }

        std::string ToString(void)
        {
            std::string str("Bottle #");

            str += std::to_string(ID);
            str += " : " + GetCurrentItem().name;

            return (str);
        }

        const  u32 ID;
        u8     SelectedItem;

    private:

        u8   *_address;
    };

    Bottle    g_bottles[3] = 
    {
        Bottle(1, 0x005879F6),
        Bottle(2, 0x005879F7),
        Bottle(3, 0x005879F8)
    };

    void    BottleSettings(MenuEntry *entry)
    {
        Bottle      *bottle = reinterpret_cast<Bottle*>(entry->GetArg());
        Keyboard    keyboard;
        std::vector<std::string>    items;

        for (const Item &item : g_bottleItems)
            if (item.id != 0)
                items.push_back(item.name);

        keyboard.DisplayTopScreen = false;
        keyboard.Populate(items);

        int choice = keyboard.Open();

        if (choice != -1)
        {
            *bottle = g_bottleItems[choice];
        }

        entry->Name() = bottle->ToString();
    }

    void    BottleManager(MenuEntry *entry)
    {
        Bottle *bottle = reinterpret_cast<Bottle*>(entry->GetArg());

        bottle->WriteItem();
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

    using   FsTryOpenFileType = u32(*)(u32, const u16*, u32);
    extern u32          g_FsTryOpenFileAddress;
    static Hook         g_FsTryOpenFileHook;
    static u32 FsTryOpenFileCallback(u32 a1, const u16 *filename, u32 mode);
    void      FindFunction(u32 &FsTryOpenFile);

    void      ForceMapsLoading(MenuEntry *entry)
    {
        // If we must enable the hook
        if (entry->WasJustActivated())
        {
            // If hook is not initialized
            if (!g_FsTryOpenFileHook.isInitialized)
            {
                // Hook on OpenFile
                u32 FsTryOpenFileAddress = g_FsTryOpenFileAddress;

                if (!FsTryOpenFileAddress)
                    FindFunction(FsTryOpenFileAddress);

                // Check that we found the function
                if (FsTryOpenFileAddress)
                {
                    g_FsTryOpenFileHook.Initialize(FsTryOpenFileAddress, (u32)FsTryOpenFileCallback);
                }
            }

            // Enable the hook
            g_FsTryOpenFileHook.Enable();
            return;
        }

        // If we must disable the hook
        if (!entry->IsActivated())
            g_FsTryOpenFileHook.Disable();
    }

    struct Map
    {
        const u16   *path;
        const char  *name;
    };

    std::vector<Map>   g_maps =
    {
        { (const u16 *)u"rom:/scene/link_info.zsi", "Link's TreeHouse" },
        { (const u16 *)u"rom:/scene/spot04_info.zsi", "Kokiri forest" },
        { (const u16 *)u"rom:/scene/spot10_info.zsi", "Kokiri - Hyrule Bridge" },
        { (const u16 *)u"rom:/scene/spot00_info.zsi", "Hyrule Field" }
    };

    u16     *strstr16(const u16 *haystack, const u16 *needle)
    {
        if (!needle || !haystack)
            return ((u16 *)haystack);

        while (*haystack)
        {
            const u16 *src = haystack;
            const u16 *pattern = needle;

            while (*src && *pattern && *src++ == *pattern++);

            if (!*pattern)
                return ((u16 *)haystack);

            haystack++;
        }

        return (nullptr);
    }

    u32     strcmp16(const u16 *s1, const u16 *s2)
    {
        while (*s1 && *s2 && *s1++ == *s2++);

        return (*s1 - *s2);
    }

    static u32 FsTryOpenFileCallback(u32 a1, const u16 *filename, u32 mode)
    {
        if (strstr16(filename, (u16 *)u".zsi") != nullptr)
        {
            for (Map &map : g_maps)
            {
                if (strcmp16(map.path, filename) == 0)
                {
                    filename = g_maps[0].path;
                    break;
                }
            }
        }

        return (((FsTryOpenFileType)g_FsTryOpenFileHook.returnCode)(a1, filename, mode));
    }

    PluginMenu  *g_menu;
    MenuFolder  *g_folder = new MenuFolder("Action Replay");

    struct AREntry
    {
        ARCodeVector     arcodes;
        u32     Storage[2];
    };

    void    ExecuteARCodes(MenuEntry *entry)
    {
        AREntry *ar = static_cast<AREntry *>(entry->GetArg());

        ARHandler::Execute(ar->arcodes, ar->Storage);
    }

    MenuEntry *CreateAREntry(const std::string &name, ARCodeVector &arcodes)
    {
        MenuEntry *entry = new MenuEntry(name, ExecuteARCodes);

        AREntry *ar = new AREntry;

        ar->Storage[0] = 0;
        ar->Storage[1] = 0;
        ar->arcodes = arcodes;

        entry->SetArg(ar);
        return (entry);
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }
    /*
    void    LogEntry(void)
    {
        File file;

        File::Open(file, "Log.txt", File::WRITE | File::SYNC | File::APPEND | File::CREATE);

        MenuFolderImpl *folder = g_folder->_item.get();
        for (int i = 0; i < folder->ItemsCount(); i++)
        {
            MenuEntry *entry = (*folder)[i]->AsMenuEntryImpl().AsMenuEntry();
            AREntry *ar = (AREntry *)entry->GetArg();

            file.WriteLine(Utils::Format("[%s]", entry->Name().c_str()));
            for (const ARCode &code : ar->arcodes)
                file.WriteLine(code.ToString());
            file.WriteLine(" ");
        }
    }*/


    void    LineReadTest(MenuEntry *entry)
    {

        File        file;
        LineReader  reader(file);

        if (File::Open(file , "cheats.txt") == 0)
        {
            std::string line;

            std::string     name;
            ARCodeVector    arcodes;

            bool error;
            bool ecode = false;
            int  count = 0;
            int index = 0;

            while (reader(line))
            {
                if (line.empty())
                {
                    if (!name.empty() && !arcodes.empty())
                        *g_folder += CreateAREntry(name, arcodes);
                    arcodes.clear();
                    name.clear();
                    ecode = false;
                    continue;
                }
                // If we're in emode
                if (ecode)
                {
                    u32  *data = reinterpret_cast<u32 *>(&arcodes.back().Data[index]);
                    std::string lStr = line.substr(0, 8);
                    std::string rStr = line.substr(9, 8);
                    u32 left = static_cast<u32>(std::stoul(lStr, nullptr, 16));
                    u32 right = static_cast<u32>(std::stoul(rStr, nullptr, 16));

                    *data++ = left;
                    *data = right;
                    index += 8;
                    count--;
                    if (count == 0)
                    {
                        ecode = false;
                    }
                        
                    continue;
                }

                // If we found a name pattern
                if (line[0] == '[')
                {
                    if (!name.empty() && !arcodes.empty())
                        *g_folder += CreateAREntry(name, arcodes);
                    arcodes.clear();
                    name.clear();

                    rtrim(line);
                    name = line.substr(1, line.size() - 2);
                    continue;
                }

                ARCode code(line, error);

                if (error)
                {
                    OSD::Notify("Error: " + line);
                    break;
                }               

                if (code.Type == 0xE0)
                {
                    ecode = true;
                    count = code.Right / 8 + (code.Right % 8 != 0 ? 1 : 0);
                    code.Data = new u8[count * 8];
                    index = 0;
                }

                arcodes.push_back(code);
            }

            if (!name.empty() && !arcodes.empty())
                *g_folder += CreateAREntry(name, arcodes);

            MessageBox(Utils::Format("Found %d cheats !", g_folder->ItemsCount()))();

           // LogEntry();
            //entry->Disable();
        }
    }

    void    Corrupter(MenuEntry *entry)
    {
        u32     memSize = 0;
        float   *address = reinterpret_cast<float *>(0x30000000);

        if (Process::CheckRegion(0x30000000, memSize))
        {
            while (memSize--)
            {
                float value = *address++;
                if (value >= 1.0f && value < 2.0f)
                {
                    int rng = Utils::Random(0, 20);
                    Process::WriteFloat(reinterpret_cast<u32>(address), 0.1f * rng);
                }
            }
        }
    }


    void    MyCheat(MenuEntry *entry)
    {
        // The osd callback
        auto osd = [](const Screen &screen)
        {
            // Only draw if the screen is the top screen;
            if (!screen.IsTop)
                return (false); ///< We didn't draw a thing

            screen.Draw("What a nice callback !", 10, 10, Color::LimeGreen, Color::Blank);

            return (true); ///< We did changed the framebuffer
        };

        // If my cheat was just activated, I enable the callback
        if (entry->WasJustActivated())
            OSD::Run(osd);

        // When my cheat is disabled, I remove the callback
        if (!entry->IsActivated())
            OSD::Remove(osd);
    }

#define C_RESET "\x18"
#define C_RED "\x1B\xFF\x01\x01"
#define C_BLUE "\x1B\x01\x01\xFF"
#define C_GREEN "\x1B\x01\xFF\x01"
#define C_ORANGE "\x1B\xFF\x45\x01"
#define C_YELLOW "\x1B\xFF\xD7\x01"

    extern "C" Handle gspGpuHandle;
#define ARVERSION 0
    int    main(void)
    {
#if ARVERSION
        g_menu = new PluginMenu("Action Replay Test", 0, 0, 1);
        PluginMenu      &menu = *g_menu;

        menu += new MenuEntry("Load cheats from file", nullptr, LineReadTest);
        menu += g_folder;

#else        
        PluginMenu      *m = new PluginMenu(C_RED "Zelda" C_GREEN " Ocarina Of Time 3D", 3, 0, 1, about);
        PluginMenu      &menu = *m;
        std::string     note;

        menu += new MenuEntry("OSD Callback", MyCheat);

        menu += new MenuEntry("L for keyboard", [](MenuEntry *entry)
        {
            if (Controller::IsKeyPressed(L))
            {
                Keyboard kb;

                u32 i;

                kb.Open(i);
            }
        });

        OSD::Notify(Color::Red << "Notification" << Color::LimeGreen << "Colored");

        menu += new MenuEntry("Clean", nullptr, [](MenuEntry *entry) {ScreenImpl::Clean(); });

        menu += new MenuEntry("Notify", [](MenuEntry *entry)
        {
           if (Controller::IsKeyPressed(X))
           {
               Color colors[10] =
               {
                   Color::Turquoise, Color::Brown, Color::Yellow, Color::Red, Color::DeepSkyBlue,
                   Color::Blank, Color::DarkGrey, Color::LimeGreen, Color::Magenta, Color::Orange
               };

               Color &fg = colors[Utils::Random(0, 9)];
               Color &bg = colors[Utils::Random(0, 9)];
               OSD::Notify("That incredibly long notification !!", fg, bg);
           }
        });

        /*
        ** Movements codes
        ********************/

        MenuFolder *folder = new MenuFolder("Movement");
            
        folder->Append(EntryWithHotkey(new MenuEntry("MoonJump", MoonJump, "Press the hotkey to be free of the gravity."), Hotkey(Key::A, "Moonjump")));
        
        note = "Use \uE077 while pressing the hotkey(s) to move very fast.\n" \
                "Be careful of the loading zone, it might put you out of bound.\n" \
                "You can change the hotkey by touching the keyboard icon on the bottom screen.";
        folder->Append(EntryWithHotkey(new MenuEntry("Fast Move \uE077 +", MoveFast, note), Hotkey(Key::ZL, "Run faster")));
        folder->Append(new MenuEntry("Epona have max carrots", EponaMaxCarrots));
        folder->Append(new MenuEntry("Epona have max carrots", EponaInfiniteCarrotsAllAreas));
        folder->Append(EntryWithHotkey(new MenuEntry("Epona MoonJump", EponaMoonJump, "Press the hotkey to be free of the gravity."), Hotkey(Key::L | Key::A, "Epona Moonjump")));

        menu.Append(folder);

        /*
        ** Battle codes
        ******************/

        folder = new MenuFolder(C_ORANGE"Battle", "Need some boosters for your fights ?");
        folder->Append(new MenuEntry("Invincible", Invincible, "With this code you'll be invincible !"));
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

        folder->Append(sword);
        folder->Append(shield);
        folder->Append(suits);
        folder->Append(items);

        folder->Append(new MenuEntry("100 Skulltulas", Skulltulas));
        folder->Append(new MenuEntry("Max Rupees", MaxRupees));

        std::string name = " : " << Color::Red << "Read the note !";

        note = "Touch the keyboard icon to set the bottle's content then activate the entry.\n\n";
        note << Color::Red << "Warning:\n"
             << Color::Orange << "Don't set the Locked value when the bottle is attributed to a key or it'll create a duplicate in your inventory and you'll loose an inventory slot.";
        
        folder->Append(AddArg(&g_bottles[0], new MenuEntry("Bottle #1" + name, BottleManager, BottleSettings, note)));
        folder->Append(AddArg(&g_bottles[1], new MenuEntry("Bottle #2" + name, BottleManager, BottleSettings, note)));
        folder->Append(AddArg(&g_bottles[2], new MenuEntry("Bottle #3" + name, BottleManager, BottleSettings, note)));




        menu.Append(folder);

        /*
        Time codes
        ***********/
        folder = new MenuFolder("Time");

        folder->Append(EntryWithHotkey(new MenuEntry("Sunrise", Sunrise), Hotkey(Key::R | Key::DPadLeft, "Sunrise")));
        folder->Append(EntryWithHotkey(new MenuEntry("Daytime", Daytime), Hotkey(Key::R | Key::DPadUp, "Daytime")));
        folder->Append(EntryWithHotkey(new MenuEntry("Sunset", Sunset), Hotkey(Key::R | Key::DPadRight, "Sunset")));
        folder->Append(EntryWithHotkey(new MenuEntry("Night", Night), Hotkey(Key::R | Key::DPadDown, "Night")));
        folder->Append(new MenuEntry("Sky is always raining", AlwaysRaining));
        menu.Append(folder);

        /*
        ** Quest
        ************/

        folder = new MenuFolder("Quest");
        {
            MenuFolder &quest = *folder;

            quest += new MenuEntry("Unlock All Best Equipment", UnlockAllBest);
            quest += new MenuEntry("Unlock All Stones", UnlockAllStones);
            quest += new MenuEntry("Unlock All Medallions", UnlockAllMedallions);
            quest += new MenuEntry("Unlock Enhanced Defense", UnlockEnhancedDefense);
            quest += new MenuEntry("Unlock Heart Pieces", UnlockHeartpieces);
            quest += new MenuEntry("Infinite Small Dungeon Keys", InfiniteSmallKeysAllDungeons);
            quest += new MenuEntry("Unlock Map and Boss Key of all Dungeons", HaveMapCompassAndBossKeyAllDungeon);

            menu += folder;
        }

        /*
        ** Misc codes
        *************/
        menu += new MenuFolder("Misc.", std::vector<MenuEntry *>(
        {
            new MenuEntry("Giant Link", GiantLink),
            new MenuEntry("Normal Link", NormalLink),
            new MenuEntry("Mini Link", MiniLink),
            new MenuEntry("Paper Link", PaperLink),
            new MenuEntry("Link always have his child voice", AlwaysChildLinkVoice),
            new MenuEntry("Link always have his adult voice", AlwaysAdultLinkVoice),
            new MenuEntry("Open Chest Many Times", OpenAnyChestInTheGameAsManyTimes),
            new MenuEntry("Collect Heart Piece Many Times", CollectHeartPiecesInOverworldAsMany),
            new MenuEntry("No Damage From Falling", NeverTakeDamageFromFalling),
            new MenuEntry("Giant knife won't break", GiantsKnifeNeverBreaks)
        }));
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

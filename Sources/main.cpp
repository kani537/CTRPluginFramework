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

namespace CTRPluginFramework
{    
    // This function is called on the plugin starts, before main
    void    PatchProcess(void)
    {

    }



    // Function to pass to plugin to decode the About text
    /*void    Decoder(std::string &output, void *input)
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
    }*/

    //  
    std::string about = u8"\n" \
        u8"Plugin for Zelda Ocarina Of Time, V3.0\n\n"
        u8"Most of these codes comes from Fort42 so a huge thanks to their original creator !!\n\n" \
        u8"GBATemp's release thread: goo.gl/Rz1uhj";

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

    void    CheatsGameFunc(MenuEntry *entry)
    {
        
    }

    using KeyVector = std::vector<Key>;

    class   KeySequence
    {
    public:

        KeySequence(KeyVector sequence) : 
        _sequence(sequence), _indexInSequence(0)
        {            
        }

        ~KeySequence(){}

        /**
         * \brief Check the sequence
         * \return true if the sequence is completed, false otherwise
         */
        bool  operator()(void)
        {
            if (Controller::IsKeyDown(_sequence[_indexInSequence]))
            {
                _indexInSequence++;

                if (_indexInSequence >= _sequence.size())
                {
                    _indexInSequence = 0;
                    return (true);
                }

                _timer.Restart();
            }

            if (_timer.HasTimePassed(Seconds(1.f)))
            {
                _indexInSequence = 0;
                _timer.Restart();
            }

            return (false);
        }

    private:
        KeyVector   _sequence;
        Clock       _timer;
        int         _indexInSequence;
        
    };

    extern u32 g_framebuffer;

    int    main(void)
    {
        PluginMenu      *m = new PluginMenu("Zelda Ocarina Of Time 3D", 3, 0, 1, about);// g_encAbout, Decoder);
        PluginMenu      &menu = *m;
        std::string     note;

        /*
        ** Movements codes
        ********************/

        menu.Append(new MenuEntry("Notify", [](MenuEntry *entry)
        {
            static KeySequence keySequence({ Key::DPadUp, Key::DPadDown, Key::L });

            if (keySequence())
            {
                OSD::Notify("This is a notification");
            }

            if (Controller::IsKeyDown(L) && Controller::IsKeyPressed(DPadUp))
                if (g_framebuffer > 0)
                    g_framebuffer--;
            if (Controller::IsKeyDown(L) && Controller::IsKeyPressed(DPadDown))
                g_framebuffer++;
        },
        [](MenuEntry *entry)
        {
            char buf[0x100];

            OSDParams params;

            NTR::FetchOSDParams(params);

            sprintf(buf, "Bottom: %d\nStride: %d\nFormat: %d, Addresse: %08X\nOffset: %d", params.isBottom, params.stride, params.format, params.leftFramebuffer,g_framebuffer);
            (MessageBox(buf))();
        }));

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

        folder = new MenuFolder("Battle", "Need some boosters for your fights ?");
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

        std::string name = " : Read the note !";
        note = "Touch the keyboard icon to set the bottle's content then activate the entry.\n\nWarning: don't set the Locked value when the bottle is attributed to a key or it'll create a duplicate in your inventory and you'll loose an inventory slot.";
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

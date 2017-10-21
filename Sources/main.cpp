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
#include "ctrulib/gpu/gx.h"
#include <memory>
#include "OSDManager.hpp"

namespace CTRPluginFramework
{
    // This function is called on the plugin starts, before main
    void    PatchProcess(void) {}

    std::string about = u8"\n" \
        u8"Plugin for Zelda Ocarina Of Time, V3.0\n\n"
        u8"Most of these codes comes from Fort42 so a huge thanks to their original creator !!\n\n" \
        u8"GBATemp's release thread: goo.gl/Rz1uhj";

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

    PluginMenu  *g_menu;
    MenuFolder  *g_folder = new MenuFolder("Action Replay");

    MenuFolder  *g_f = new MenuFolder("That folder !");
    MenuFolder  *g_f2 = nullptr;

    void    f(void)
    {
        MessageBox(Color::Yellow << "Title", "Wow, awesome message !")();
        MessageBox("Wow, awesome message !")();
    }

#define ARVERSION 1
    void    LineReadTest(MenuEntry *entry);
    int     main(void)
    {
#if ARVERSION
        g_menu = new PluginMenu("Action Replay Test", 0, 0, 1);
        PluginMenu      &menu = *g_menu;

        menu += new MenuEntry("Load cheats from file", nullptr, LineReadTest);
        menu += g_folder;

        menu += g_f;

        menu.OnFirstOpening = f;

        menu += new MenuEntry("Test", nullptr, [](MenuEntry *entry)
        {
            Keyboard kb({ "Add entry", "Remove entry", "Add entries", "Clear", "Remove range" });

            kb.CanAbort(false);
            switch (kb.Open())
            {
                case 0:
                    if (g_f2 == nullptr)
                        g_f2 = new MenuFolder("Whatttt");
                    *g_f += g_f2;
                    break;
                case 1:
                    if (g_f2 != nullptr)
                        *g_f -= g_f2;   
                    break;
                case 2:
                    for (int i = 0; i < 10; i++)
                        *g_f += new MenuEntry(Utils::Format("#%d", i));
                    break;
                case 3:
                    if (g_f2 != nullptr)
                        *g_f -= g_f2;
                    g_f->Clear();
                    break;
                case 4:
                    g_f->Remove(1, 2, false);
                    break;
            default:
                break;
            }
        });

#else        
        PluginMenu      *m = new PluginMenu(C_RED "Zelda" C_GREEN " Ocarina Of Time 3D", 3, 0, 1, about);
        PluginMenu      &menu = *m;
        std::string     note;

        InitPointerChecker(menu);

        OSD::Run(g_osdcb);

        
        menu += []
        {
            static Clock timer;

            float delta = timer.Restart().AsSeconds();

            g_frameTime = delta;
            g_eps = (u32)(g_second / delta);
        };
        menu += new MenuEntry("Wait for frame", [](MenuEntry *entry)
        {
            
            auto osd = [](const Screen &screen)
            {
                
                if (!screen.IsTop)
                {
                    svcSignalEvent(g_event);
                    screen.Draw("OSD", 10, 20);
                    return (true);
                }
                return (false);
            };

            if (entry->WasJustActivated())
            {
                if (g_event == 0)
                    svcCreateEvent(&g_event, RESET_ONESHOT);
                OSD::Run(osd);
                //OSD::Run(g_osdcb);
                svcClearEvent(g_event);
            }

            if (!entry->IsActivated())
            {
                OSD::Stop(osd);
            }
            else
            {
                svcWaitSynchronization(g_event, U64_MAX);
                svcClearEvent(g_event);
            }            
        });

        menu += new MenuEntry("L for keyboard", [](MenuEntry *entry)
        {
            static u32  i = 0;
            auto osd = [&](const Screen &screen)
            {
                if (!screen.IsTop) return (false);

                screen.Draw("i val: " << Color::Green << i, 10, 10);
                return (true);
            };

            ONCE(OSD::Run(osd));
            if (Controller::IsKeyPressed(L))
            {
                Keyboard    kb;

                kb.Open(i);               
            }

            if (!entry->IsActivated())
            {
                RESET(OSD::Stop(osd));
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
        menu += new MenuFolder("Misc.", "Various cheats",
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
        });
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

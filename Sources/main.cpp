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
            OSD::Stop(osd);
    }

    void    f(MenuEntry *entry)
    {
        if (Controller::IsKeyPressed(Key::X))
        {
            // Pause the process
            Process::Pause();

            u32         val = 0;
            Keyboard    kb(Color::Green << "That keyboard !\n\n" << ResetColor() << "Enter a value");
            MessageBox  msgBox(Color::Yellow << "Warning\n\n" << ResetColor() << "You did something worthy of a warning");

            // Open the keyboard
            kb.Open(val); ///< Screen will be cleaned on exit

            // Open the messagebox
            msgBox(); ///< Screen will be cleaned on exit

            // Resume the process
            Process::Play();
        }
    }

#define C_RESET "\x18"
#define C_RED "\x1B\xFF\x01\x01"
#define C_BLUE "\x1B\x01\x01\xFF"
#define C_GREEN "\x1B\x01\xFF\x01"
#define C_ORANGE "\x1B\xFF\x45\x01"
#define C_YELLOW "\x1B\xFF\xD7\x01"

#define ONCE(expr) static bool __isAlreadyAdded = false;\
    if (!__isAlreadyAdded) {\
        (expr);\
        __isAlreadyAdded = true;}

#define RESET(expr) if (__isAlreadyAdded) {\
    (expr);\
    __isAlreadyAdded = false;}
        
    extern "C" Handle gspGpuHandle;
    Time    t_second = Seconds(1.f);
    float g_second = Seconds(1.f).AsSeconds();
    float g_frameTime = 0.f;
    u32   g_eps = 0;


    static Handle g_event = 0;

    auto g_osdcb = [](const Screen &screen)
    {
        if (screen.IsTop)
        {
            std::string str = "FrameTime: " << Color::Green << Utils::Format("%.02f", g_frameTime);

            screen.Draw(str, 10, 10);

            str = "Executed: " << Color::Green << g_eps << "/s";
            screen.Draw(str, 10, 20);
            svcSignalEvent(g_event);
            return (true);
        }
        return (false);
    };

    template <typename T>
    T   *GetArg(MenuEntry *entry, T defaultValue)
    {
        T   *arg = reinterpret_cast<T *>(entry->GetArg());

        if (arg == nullptr)
        {
            arg = new T(defaultValue);
            entry->SetArg(arg);
        }

        return (arg);
    }

    enum
    {
        U8,
        U16,
        U32,
        Float
    };

    u32         g_mode = U32;

    void    UpdateEntries(void);

    MenuFolder  *g_pointerFolder = new MenuFolder("Pointers testing", "",
    {
        new MenuEntry("Change mode", nullptr, [](MenuEntry *entry)
        {
            Keyboard kb(std::vector<std::string>({"u8 ", "u16", "u32", "Float"}));

            kb.CanAbort(false);
            g_mode = kb.Open();
            UpdateEntries();
        })
    });

    std::string FormatAddress(u32 addr, u32 off)
    {
        std::string address = Utils::Format("%08X", addr == 0 ? off : addr);
        std::string offset = Utils::Format("%X", off);
        
        if (addr == 0)
        {
            return ("[" << Color::Yellow << address << ResetColor() << "] = ");
        }

        return ("[" << Color::Yellow << address << ResetColor() 
            << " + " << Color::Orange << offset << ResetColor() << "] = ");
    }

    std::string FormatValue(u32 &address)
    {
        union
        {

            u32 U32;
            float Float;
            u8 U8;
            u16 U16;
        }val = { 0 };

        std::string ret; 

        if (!Process::Read32(address, address))
            address = 0;

        val.U32 = address;
        
        switch (g_mode)
        {
        case U8:
            ret = Color::SkyBlue << Utils::Format("%08X", address) 
            << ResetColor() << " => " 
            << Color::Green << Utils::Format("%02X", val.U8);
            break;
        case U16:
            ret = Color::SkyBlue << Utils::Format("%08X", address)
                << ResetColor() << " => "
                << Color::Green << Utils::Format("%04X", val.U16);
            break;
        case U32:
            ret = Color::Green << Utils::Format("%08X", val.U32);
            break;
        case Float:
            ret = Color::SkyBlue << Utils::Format("%08X", address)
                << ResetColor() << " => "
                << Color::Green << Utils::Format("%.02f", val.Float);
            break;
        }

        return (ret);
    }

    void    UpdateEntries(void)
    {
        std::vector<MenuEntry *> &&list = g_pointerFolder->GetEntryList();

        u32 address = 0;

        for (int i = 1; i < 11; i++)
        {
            MenuEntry *entry = list[i];
            std::string &name = entry->Name();

            u32 val = *GetArg<u32>(entry, 0);

            name = FormatAddress(address, val);

            address += val;

            name += FormatValue(address);
        }
    }

    void    ChangeValue(MenuEntry *entry)
    {
        u32 *val = GetArg<u32>(entry, 0);

        Keyboard kb;

        kb.CanAbort(false);
        kb.Open(*val);
        UpdateEntries();
    }

    void    InitPointerChecker(PluginMenu &menu)
    {
        menu += g_pointerFolder;

        for (int i = 0; i < 10; i++)
            *g_pointerFolder += new MenuEntry("", nullptr, ChangeValue);

        menu += []
        {
            static Clock timer;

            if (timer.HasTimePassed(Seconds(1.f)))
            {
                UpdateEntries();
                timer.Restart();
            }
        };

        UpdateEntries();
    }


    class Pointer
    {
    public:
        
        static u32     Get(u32 base, u32 off1);
        static u32     Get(u32 base, u32 off1, u32 off2);
        static u32     Get(u32 base, u32 off1, u32 off2, u32 off3);
        static u32     Get(u32 base, u32 off1, u32 off2, u32 off3, u32 off4);
        static u32     Get(u32 base, u32 off1, u32 off2, u32 off3, u32 off4, u32 off5);
    };

    #define Read32(addr, out) if (!Process::Read32((addr), (out))) goto error;

    u32     Pointer::Get(u32 base, u32 off1)
    {
        Read32(base, base);
        return (base + off1);
     error:
        return (0);
    }

    u32     Pointer::Get(u32 base, u32 off1, u32 off2)
    {
        Read32(base, base);
        Read32(base + off1, base);
        return (base + off2);
    error:
        return (0);
    }

    u32     Pointer::Get(u32 base, u32 off1, u32 off2, u32 off3)
    {
        Read32(base, base);
        Read32(base + off1, base);
        Read32(base + off2, base);
        return (base + off3);
    error:
        return (0);
    }

    u32     Pointer::Get(u32 base, u32 off1, u32 off2, u32 off3, u32 off4)
    {
        Read32(base, base);
        Read32(base + off1, base);
        Read32(base + off2, base);
        Read32(base + off3, base);
        return (base + off4);
    error:
        return (0);
    }

    u32     Pointer::Get(u32 base, u32 off1, u32 off2, u32 off3, u32 off4, u32 off5)
    {
        Read32(base, base);
        Read32(base + off1, base);
        Read32(base + off2, base);
        Read32(base + off3, base);
        Read32(base + off4, base);
        return (base + off5);
    error:
        return (0);
    }

    void    f(void)
    {
        u32 val = Pointer::Get(0x12345678, 0x1, 0x2);
        u32 val2 = Pointer::Get(0x12345678, 0x1, 0x2, 0x3);
    }
    
    static bool     g_sync = false;
    static u64 timeout = (Seconds(1.f) / 30.f).AsMicroseconds() * 1000;
    Time    g_limit = Seconds(Seconds(1.f).AsSeconds() / 60.f);
    vu32    g_counter = 0;
    u32     g_iter = 0;
    //float   g_second = Seconds(1.f).AsSeconds();
    Clock   timer;
    static u32 index = 0;
    static const std::string keys[] =
    { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"
    };

    void    func(MenuEntry *entry)
    {
        u32 value = 0x12345678;
        
        // Our debug string with: onTopScreen, str, posX, posY
        OSDManager["fdebug"] = { true, Utils::Format("%08X", value), 10, 10 };

        // Remove the debug string on disable
        if (!entry->IsActivated())
            OSDManager.Remove("fdebug");
    }

    void    func2(MenuEntry *entry)
    {
        // On first activation, I set my debug parameters
        if (entry->WasJustActivated())
        {
            OSDManager["func2Debug"].SetScreen(true).SetPos(10, 10) = "Empty string... Or not";
            // This is the same as:
            // OSDManager["func2Debug"] = {true, "Empty string... Or not", 10, 10};
            // Just more verbose
        }

        // When the entry is disabled, I remove the entry from the manager
        if (!entry->WasJustActivated())
        {
            OSDManager.Remove("func2Debug");
        }

        u32 value = 10;
        // Then I can display whatever I want
        OSDManager["func2Debug"] = Color::Red << "Value: " << Color::Green << value;
    }

#define ARVERSION 1
    int    main(void)
    {
#if ARVERSION
        g_menu = new PluginMenu("Action Replay Test", 0, 0, 1);
        PluginMenu      &menu = *g_menu;

        menu += new MenuEntry("Load cheats from file", nullptr, LineReadTest);
        menu += g_folder;

        //OSD::Run(g_osdcb);

        /*menu += []
        {
            static Clock timer;

            Time d = timer.Restart();
            float delta = d.AsSeconds();
            
            g_frameTime = delta;
            g_eps = (u32)(g_second / delta);
            Sleep(g_limit - d);
        };
        menu += new MenuEntry("Wait for frame", [](MenuEntry *entry)
        {

            if (entry->WasJustActivated())
            {
                if (g_event == 0)
                    svcCreateEvent(&g_event, RESET_ONESHOT);
                g_sync = true;
                svcClearEvent(g_event);
            }

            if (!entry->IsActivated())
            {
                g_sync = false;
            }
            else
            {
                svcWaitSynchronization(g_event, timeout);
                //svcClearEvent(g_event);
            }
        });*/

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
            if (!g_sync)
                Sleep(Milliseconds(5));
        };

        // Launch menu and mainloop
        menu.Run();

        // Exit plugin
        return (0);
    }
}

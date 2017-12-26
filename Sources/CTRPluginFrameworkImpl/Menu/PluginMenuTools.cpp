#include "CTRPluginFrameworkImpl/Menu/HotkeysModifier.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuTools.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryTools.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "CTRPluginFramework/Utils/StringExtensions.hpp"
#include "Hook.hpp"
#include "CTRPluginFramework/System/Sleep.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFramework/Graphics/OSD.hpp"
#include <ctime>
#include <cstring>
#include <cstdio>
#include "ctrulib/srv.h"

namespace CTRPluginFramework
{
    enum Mode
    {
        NORMAL = 0,
        ABOUT,
        HEXEDITOR,
        FREECHEATS,
        GWRAMDUMP,
        MISCELLANEOUS,
        SETTINGS
    };

    static int  g_mode = NORMAL;

    PluginMenuTools::PluginMenuTools(std::string &about, HexEditor &hexEditor, FreeCheats &freeCheats) :
        _about(about),
        _mainMenu("Tools"),
        _miscellaneousMenu("Miscellaneous"),
        _settingsMenu("Settings"),
        _freecheatsEntry(nullptr),
        _hexEditorEntry(nullptr),
        _hexEditor(hexEditor),
        _freeCheats(freeCheats),
        _menu(&_mainMenu, nullptr),
        _abouttb("About", _about, IntRect(30, 20, 340, 200)),
        _exit(false)
    {
        InitMenu();
    }

    static void    MenuHotkeyModifier(void)
    {
        u32 keys = Preferences::MenuHotkeys;

        (HotkeysModifier(keys, "Select the hotkeys you'd like to use to open the menu."))();

        if (keys != 0)
            Preferences::MenuHotkeys = keys;
    }

    void    PluginMenuTools::UpdateSettings(void)
    {
        if (Preferences::UseFloatingBtn) _settingsMenu[1]->AsMenuEntryImpl().Enable();
        else _settingsMenu[1]->AsMenuEntryImpl().Disable();

        if (Preferences::AutoSaveCheats) _settingsMenu[2]->AsMenuEntryImpl().Enable();
        else _settingsMenu[2]->AsMenuEntryImpl().Disable();

        if (Preferences::AutoSaveFavorites) _settingsMenu[3]->AsMenuEntryImpl().Enable();
        else _settingsMenu[3]->AsMenuEntryImpl().Disable();

        if (Preferences::AutoLoadCheats) _settingsMenu[4]->AsMenuEntryImpl().Enable();
        else _settingsMenu[4]->AsMenuEntryImpl().Disable();

        if (Preferences::AutoLoadFavorites) _settingsMenu[5]->AsMenuEntryImpl().Enable();
        else _settingsMenu[5]->AsMenuEntryImpl().Disable();

        if (Preferences::DrawTouchCursor) _miscellaneousMenu[2]->AsMenuEntryTools().Enable();
        else _miscellaneousMenu[2]->AsMenuEntryTools().Disable();

        if (Preferences::ShowTopFps) _miscellaneousMenu[3]->AsMenuEntryTools().Enable();
        else _miscellaneousMenu[3]->AsMenuEntryTools().Disable();

        if (Preferences::ShowBottomFps) _miscellaneousMenu[4]->AsMenuEntryTools().Enable();
        else _miscellaneousMenu[4]->AsMenuEntryTools().Disable();
    }

    using   FsTryOpenFileType = u32(*)(u32, u16*, u32);

    enum HookFilesMode
    {
        NONE = 0,
        OSD = 1,
        FILE = 2
    };
    static Hook         g_FsTryOpenFileHook;
    static u32          g_HookMode = NONE;
    static File         g_hookExportFile;
    static char         *g_buffer = nullptr;
    static char         *g_buffer2 = nullptr;
    u32                 g_FsTryOpenFileAddress = 0;
    char                **g_filenames = nullptr;
    int                 g_index = 0;
    LightLock           g_OpenFileLock;

    static u32      FindNearestSTMFD(u32 addr)
    {
        for (u32 i = 0; i < 1024; i++)
        {
            addr -= 4;
            if (*(u16 *)(addr + 2) == 0xE92D)
                return addr;
        }
        return (0);
    }

    static void      FindFunction(u32 &FsTryOpenFile)
    {
        const u8 tryOpenFilePat1[] = { 0x0D, 0x10, 0xA0, 0xE1, 0x00, 0xC0, 0x90, 0xE5, 0x04, 0x00, 0xA0, 0xE1, 0x3C, 0xFF, 0x2F, 0xE1 };
        const u8 tryOpenFilePat2[] = { 0x10, 0x10, 0x8D, 0xE2, 0x00, 0xC0, 0x90, 0xE5, 0x05, 0x00, 0xA0, 0xE1, 0x3C, 0xFF, 0x2F, 0xE1 };

        u32    *addr = (u32 *)0x00100000;
        u32    *maxAddress = (u32 *)(Process::GetTextSize() + 0x00100000);

        while (addr < maxAddress)
        {
            if (!memcmp(addr, tryOpenFilePat1, sizeof(tryOpenFilePat1)) || !memcmp(addr, tryOpenFilePat2, sizeof(tryOpenFilePat2)))
            {
                FsTryOpenFile = FindNearestSTMFD((u32)addr);
                break;
            }
            addr++;
        }
    }

    static u32      FsTryOpenFileCallback(u32 a1, u16 *fileName, u32 mode)
    {
        if (g_HookMode & OSD)
        {
            while (g_index >= 50)
                Sleep(Microseconds(1));

            LightLock_Lock(&g_OpenFileLock);

            u8  *pname = (u8 *)g_filenames[g_index++];
            u16 *name = fileName;
            int i = 0;
            while (*name && i++ < 79)
                *pname++ = (u8)*name++;

            *pname = 0;

            LightLock_Unlock(&g_OpenFileLock);
        }

        if (g_HookMode & FILE)
        {
            u16 *name = fileName;
            u8  *u8Name = (u8 *)g_buffer2;
           // u8  *pBuf = (u8 *)g_buffer;

            while (*name)
                *u8Name++ = (u8)*name++;
            *u8Name++ = '\r';
            *u8Name++ = '\n';
            *u8Name = 0;

            g_hookExportFile.Write(g_buffer2, strlen(g_buffer2));
        }

        return (((FsTryOpenFileType)g_FsTryOpenFileHook.returnCode)(a1, fileName, mode));
    }

    static void    _DisplayLoadedFiles(MenuEntryTools *entry)
    {
        // If we must enable the hook
        if (entry->IsActivated())
        {
            // If buffer is null, allocate it
            if (g_filenames == nullptr)
            {
                g_filenames = new char*[50];
                for (int i = 0; i < 50; i++)
                    g_filenames[i] = new char[80];
            }

            // If hook is not initialized
            if (!g_FsTryOpenFileHook.flags.isInitialized)
            {
                // Hook on OpenFile
                u32 FsTryOpenFileAddress = 0;

                FindFunction(FsTryOpenFileAddress);

                // Check that we found the function
                if (FsTryOpenFileAddress)
                {
                    LightLock_Init(&g_OpenFileLock);

                    g_FsTryOpenFileHook.Initialize(FsTryOpenFileAddress, (u32)FsTryOpenFileCallback);
                    g_FsTryOpenFileAddress = FsTryOpenFileAddress;
                }
                else
                {
                    OSD::Notify("Error: couldn't find OpenFile function");
                    Preferences::DisplayFilesLoading = false;
                    return;

                }
            }

            // Enable the hook
            g_FsTryOpenFileHook.Enable();
            Preferences::DisplayFilesLoading = true;
            g_HookMode |= OSD;
            return;
        }

        // If we must disable the hook
        Preferences::DisplayFilesLoading = false;
        g_HookMode &= OSD;

        if (!g_HookMode)
            g_FsTryOpenFileHook.Disable();
    }

    static void    _WriteLoadedFiles(MenuEntryTools *entry)
    {
        // If we must enable the hook
        if (entry->IsActivated())
        {

            // If buffers aren't allocated
            if (g_buffer == nullptr)
                g_buffer = new char[256];
            if (g_buffer2 == nullptr)
                g_buffer2 = new char[256];

            // If hook is not initialized
            if (!g_FsTryOpenFileHook.flags.isInitialized)
            {
                // Hook on OpenFile
                u32 FsTryOpenFileAddress = 0;

                FindFunction(FsTryOpenFileAddress);

                // Check that we found the function
                if (FsTryOpenFileAddress)
                {
                    LightLock_Init(&g_OpenFileLock);

                    g_FsTryOpenFileHook.Initialize(FsTryOpenFileAddress, (u32)FsTryOpenFileCallback);
                    g_FsTryOpenFileAddress = FsTryOpenFileAddress;
                }
                else
                {
                    OSD::Notify("Error: couldn't find OpenFile function");
                    return;
                }
            }

            // Open the file
            int mode = File::READ | File::WRITE | File::CREATE | File::APPEND;
            if (File::Open(g_hookExportFile, "LoadedFiles.txt", mode) != 0)
            {
                OSD::Notify("Error: couldn't open LoadedFiles.txt", Color::Red, Color::Blank);
                return;
            }

            // Enable the hook
            g_FsTryOpenFileHook.Enable();
            g_HookMode |= FILE;
            return;
        }

        // If we must disable the hook
        g_HookMode &= FILE;
        g_hookExportFile.Flush();
        g_hookExportFile.Close();
        if (!g_HookMode)
            g_FsTryOpenFileHook.Disable();
    }

    static bool     ConfirmBeforeProceed(const std::string &task)
    {
        std::string msg = Color::Yellow << "Warning\n\n" << ResetColor() << "Do you really want to " + task + " ?";
        MessageBox  msgBox(msg, DialogType::DialogYesNo);

        return (msgBox());
    }

    static void     Shutdown(void)
    {
        if (ConfirmBeforeProceed("shutdown"))
        {
            srvPublishToSubscriber(0x203, 0);
            Sleep(Seconds(10));
        }
    }

    static void     Reboot(void)
    {
        if (ConfirmBeforeProceed("reboot"))
        {
            svcKernelSetState(7);
            Sleep(Seconds(10));
        }
    }

    void    PluginMenuTools::InitMenu(void)
    {
        // Main menu
        _mainMenu.Append(new MenuEntryTools("About", [] { g_mode = ABOUT; }, Icon::DrawAbout));
        _hexEditorEntry = new MenuEntryTools("Hex Editor", [] { g_mode = HEXEDITOR; }, Icon::DrawGrid);
        _mainMenu.Append(_hexEditorEntry);
        _freecheatsEntry = new MenuEntryTools("Free Cheats", [] { g_mode = FREECHEATS; }, Icon::DrawCentreOfGravity);
        _mainMenu.Append(_freecheatsEntry);

        _mainMenu.Append(new MenuEntryTools("Gateway RAM Dumper", [] { g_mode = GWRAMDUMP; }, Icon::DrawRAM));
        _mainMenu.Append(new MenuEntryTools("Miscellaneous", nullptr, Icon::DrawMore, new u32(MISCELLANEOUS)));
        _mainMenu.Append(new MenuEntryTools("Settings", nullptr, Icon::DrawSettings, this));
        _mainMenu.Append(new MenuEntryTools("Shutdown the 3DS", Shutdown, Icon::DrawShutdown));
        _mainMenu.Append(new MenuEntryTools("Reboot the 3DS", Reboot, Icon::DrawRestart));

        // Miscellaneous menu
        _miscellaneousMenu.Append(new MenuEntryTools("Display loaded files", _DisplayLoadedFiles, true));
        _miscellaneousMenu.Append(new MenuEntryTools("Write loaded files to file", _WriteLoadedFiles, true));
        _miscellaneousMenu.Append(new MenuEntryTools("Display touch cursor", [] { Preferences::DrawTouchCursor = !Preferences::DrawTouchCursor; }, true, Preferences::DrawTouchCursor));
        _miscellaneousMenu.Append(new MenuEntryTools("Display top screen's fps", [] {Preferences::ShowTopFps = !Preferences::ShowTopFps; }, true, Preferences::ShowTopFps));
        _miscellaneousMenu.Append(new MenuEntryTools("Display bottom screen's fps", [] {Preferences::ShowBottomFps = !Preferences::ShowBottomFps; }, true, Preferences::ShowBottomFps));

        // Settings menu
        _settingsMenu.Append(new MenuEntryTools("Change menu hotkeys", MenuHotkeyModifier, Icon::DrawGameController));
        _settingsMenu.Append(new MenuEntryTools("Use floating button", [] { Preferences::UseFloatingBtn = !Preferences::UseFloatingBtn; }, true, Preferences::UseFloatingBtn));
        _settingsMenu.Append(new MenuEntryTools("Auto save enabled cheats", [] { Preferences::AutoSaveCheats = !Preferences::AutoSaveCheats; }, true, Preferences::AutoSaveCheats));
        _settingsMenu.Append(new MenuEntryTools("Auto save favorites", [] { Preferences::AutoSaveFavorites = !Preferences::AutoSaveFavorites; }, true, Preferences::AutoSaveFavorites));
        _settingsMenu.Append(new MenuEntryTools("Auto load enabled cheats at starts", [] { Preferences::AutoLoadCheats = !Preferences::AutoLoadCheats; }, true, Preferences::AutoLoadCheats));
        _settingsMenu.Append(new MenuEntryTools("Auto load favorites at starts", [] { Preferences::AutoLoadFavorites = !Preferences::AutoLoadFavorites; }, true, Preferences::AutoSaveFavorites));
        _settingsMenu.Append(new MenuEntryTools("Load enabled cheats now", [] { Preferences::LoadSavedEnabledCheats(); }, nullptr));
        _settingsMenu.Append(new MenuEntryTools("Load favorites now", [] { Preferences::LoadSavedFavorites(); }, nullptr));

        //_settingsMenu.Append(new MenuEntryTools("Inject B when closing the menu", [] { Preferences::InjectBOnMenuClose = !Preferences::InjectBOnMenuClose; }, true));
    }

    bool    PluginMenuTools::operator()(EventList &eventList, Time &delta)
    {
        if (g_mode == HEXEDITOR)
        {
            if (_hexEditor(eventList))
                g_mode = NORMAL;
            return (false);
        }

        if (g_mode == FREECHEATS)
        {
            if (_freeCheats(eventList))
                g_mode = NORMAL;
            return (false);
        }

        if (g_mode == ABOUT)
        {
            if (!_abouttb.IsOpen())
                _abouttb.Open();
            else
                g_mode = NORMAL;
        }

        if (g_mode == GWRAMDUMP)
        {
            _gatewayRamDumper();
            g_mode = NORMAL;
            return (false);
        }

        // Process Event
        for (int i = 0; i < eventList.size(); i++)
            _ProcessEvent(eventList[i]);

        // Update
        _Update(delta);

        // Render Top
        _RenderTop();

        // Render Bottom
        _RenderBottom();

        // Check buttons

        bool exit = _exit || Window::BottomWindow.MustClose();
        _exit = false;
        return (exit);
    }

    void    PluginMenuTools::TriggerFreeCheatsEntry(bool isEnabled) const
    {
        if (!isEnabled)
        {
            _freecheatsEntry->Hide();
            FreeCheats::DisableAll();
        }
        else
            _freecheatsEntry->Show();
    }

    void PluginMenuTools::TriggerHexEditor(bool isEnabled) const
    {
        if (!isEnabled)
        {
            _hexEditorEntry->Hide();
        }
        else
            _hexEditorEntry->Show();
    }

    /*
    ** Process Event
    *****************/
    void    PluginMenuTools::_ProcessEvent(Event &event)
    {
        if (_abouttb.IsOpen())
        {
            _abouttb.ProcessEvent(event);
            return;
        }

        MenuItem    *item = nullptr;
        static int mode = 0;

        int ret = _menu.ProcessEvent(event, &item);

        if (ret == EntrySelected && item != nullptr)
        {
            void *arg = ((MenuEntryTools *)item)->GetArg();

            if (arg == this)
            {
                mode = SETTINGS;
                _menu.Open(&_settingsMenu);
            }
            else if (arg != nullptr && *(u32 *)arg == MISCELLANEOUS)
            {
                mode = MISCELLANEOUS;
                _menu.Open(&_miscellaneousMenu);
            }
        }

        if (ret == MenuClose)
        {
            if (mode == SETTINGS)
            {
                mode = 0;

                int selector = 5;

                if (!_freecheatsEntry->IsVisible()) selector--;
                if (!_hexEditorEntry->IsVisible()) selector--;
                _menu.Open(&_mainMenu, selector);
            }
            else if (mode == MISCELLANEOUS)
            {
                mode = 0;
                _menu.Open(&_mainMenu, 4);
            }
            else
            {
                _exit = true;
            }
        }
    }

    void PluginMenuTools::_RenderTopMenu(void)
    {

    }

    /*
    ** Render Top
    **************/

    void    PluginMenuTools::_RenderTop(void)
    {
        // Enable renderer
        Renderer::SetTarget(TOP);

        if(_abouttb.IsOpen())
        {
            _abouttb.Draw();
            return;
        }

      /*  // Window
        Window::TopWindow.Draw();

        // Title
        int posY = 25;
        int xx = Renderer::DrawSysString("Tools", 40, posY, 330, Color::Blank);
        Renderer::DrawLine(40, posY, xx, Color::Blank);*/

        _menu.Draw();
    }

    /*
    ** Render Bottom
    *****************/
    void    PluginMenuTools::_RenderBottom(void)
    {
        const Color    &black = Color::Black;
        const Color    &blank = Color::Blank;
        const Color    &dimGrey = Color::BlackGrey;

        // Enable renderer
        Renderer::SetTarget(BOTTOM);

        // Window
        Window::BottomWindow.Draw();

        // Draw Framework version
        {
            static const char *version = "CTRPluginFramework Beta V.0.3.5";
            static const u32 xpos = (320 - Renderer::LinuxFontSize(version)) / 2;

            int posY = 205;
            Renderer::DrawString(version, xpos, posY, blank);
        }
    }

    /*
    ** Update
    ************/
    void    PluginMenuTools::_Update(Time delta)
    {
        /*
        ** Buttons
        *************/
        bool        isTouched = Touch::IsDown();
        IntVector   touchPos(Touch::GetPosition());

        Window::BottomWindow.Update(isTouched, touchPos);
    }
}

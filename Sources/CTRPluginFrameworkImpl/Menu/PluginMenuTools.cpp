#include "CTRPluginFrameworkImpl/Menu/HotkeysModifier.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuTools.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryTools.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"

namespace CTRPluginFramework
{
    enum Mode
    {
        NORMAL = 0,
        ABOUT,
        HEXEDITOR,
        FREECHEATS,
        GWRAMDUMP,
    };

    static int  g_mode = NORMAL;

    PluginMenuTools::PluginMenuTools(std::string &about, HexEditor &hexEditor, FreeCheats &freeCheats) :
        _about(about),
        _mainMenu("Tools"),
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
        if (Preferences::AutoSaveCheats) _settingsMenu[1]->AsMenuEntryImpl().Enable();
        else _settingsMenu[1]->AsMenuEntryImpl().Disable();

        if (Preferences::AutoSaveFavorites) _settingsMenu[2]->AsMenuEntryImpl().Enable();
        else _settingsMenu[2]->AsMenuEntryImpl().Disable();

        if (Preferences::AutoLoadCheats) _settingsMenu[3]->AsMenuEntryImpl().Enable();
        else _settingsMenu[3]->AsMenuEntryImpl().Disable();

        if (Preferences::AutoLoadFavorites) _settingsMenu[4]->AsMenuEntryImpl().Enable();
        else _settingsMenu[4]->AsMenuEntryImpl().Disable();
    }

    void    PluginMenuTools::InitMenu(void)
    {
        _mainMenu.Append(new MenuEntryTools("About", [] { g_mode = ABOUT; }, Icon::DrawAbout));
        _hexEditorEntry = new MenuEntryTools("Hex Editor", [] { g_mode = HEXEDITOR; }, Icon::DrawGrid);
        _mainMenu.Append(_hexEditorEntry);
        _freecheatsEntry = new MenuEntryTools("Free Cheats", [] { g_mode = FREECHEATS; }, Icon::DrawCentreOfGravity);
        _mainMenu.Append(_freecheatsEntry);

        _mainMenu.Append(new MenuEntryTools("Gateway RAM Dumper", [] { g_mode = GWRAMDUMP; }, Icon::DrawRAM));
        _mainMenu.Append(new MenuEntryTools("Settings", nullptr, Icon::DrawSettings, this));

        _settingsMenu.Append(new MenuEntryTools("Change menu hotkeys", MenuHotkeyModifier, Icon::DrawGameController));
        _settingsMenu.Append(new MenuEntryTools("Auto save enabled cheats", [] { Preferences::AutoSaveCheats = !Preferences::AutoSaveCheats; }, true, Preferences::AutoSaveCheats));
        _settingsMenu.Append(new MenuEntryTools("Auto save favorites", [] { Preferences::AutoSaveFavorites = !Preferences::AutoSaveFavorites; }, true, Preferences::AutoSaveFavorites));
        _settingsMenu.Append(new MenuEntryTools("Auto load enabled cheats at starts", [] { Preferences::AutoLoadCheats = !Preferences::AutoLoadCheats; }, true, Preferences::AutoLoadCheats));
        _settingsMenu.Append(new MenuEntryTools("Auto load favorites at starts", [] { Preferences::AutoLoadFavorites = !Preferences::AutoLoadFavorites; }, true, Preferences::AutoSaveFavorites));
        _settingsMenu.Append(new MenuEntryTools("Load enabled cheats now", [] { Preferences::LoadSavedEnabledCheats(); }, nullptr));
        _settingsMenu.Append(new MenuEntryTools("Load favorites now", [] { Preferences::LoadSavedFavorites(); }, nullptr));

        //_settingsMenu.Append(new MenuEntryTools("Inject B when closing the menu", [] { Preferences::InjectBOnMenuClose = !Preferences::InjectBOnMenuClose; }, true));
        //_settingsMenu.Append(new MenuEntryTools("Touch Cursor", [] { Preferences::DrawTouchCursor = !Preferences::DrawTouchCursor; }, true));
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
        static bool settingsIsOpen = false;

        int ret = _menu.ProcessEvent(event, &item);

        if (ret == EntrySelected && item != nullptr && ((MenuEntryTools *)item)->GetArg() == this)
        {
            settingsIsOpen = true;
            _menu.Open(&_settingsMenu);
        }

        if (ret == MenuClose)
        {
            if (settingsIsOpen)
            {
                settingsIsOpen = false;

                int selector = 4;

                if (!_freecheatsEntry->IsVisible()) selector--;
                if (!_hexEditorEntry->IsVisible()) selector--;
                _menu.Open(&_mainMenu, selector);
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
        Color    &black = Color::Black;
        Color    &blank = Color::Blank;
        Color    &dimGrey = Color::BlackGrey;

        // Enable renderer
        Renderer::SetTarget(BOTTOM);

        // Window
        Window::BottomWindow.Draw();

        // Draw About text
        {
            int posY = 30;
            int maxY = 200;
            int posX = 30;
            int maxX = 290;

            //Renderer::DrawSysStringReturn(reinterpret_cast<const u8 *>(_about.c_str()), 
            //                              posX, posY, maxX, blank, maxY);
        }

        // Draw Framework version
        {
            
            int posY = 205;
            Renderer::DrawString((char *)"CTRPluginFramework Alpha V.0.1.2", 52, posY, blank); //40
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

#ifndef CTRPLUGINFRAMEWORK_MENU_HPP
#define CTRPLUGINFRAMEWORK_MENU_HPP

#include "Vector.hpp"
#include "Button.hpp"
#include "ToggleButton.hpp"
#include "CheckedButton.hpp"
#include "Clock.hpp"
#include "CTRPluginFramework/Graphics/TextBox.hpp"

#include <queue>

namespace CTRPluginFramework
{
    class MenuEntry;
    class MenuItem;
    class MenuFolder;
    class Event;
    class Time;
    class Clock;
    class Menu
    {
        struct MenuFlags
        {
            bool showFPS : 1;
        };

    public:

        Menu(std::string name = "Cheats", std::string note = "");
        ~Menu(void);

        void    Append(MenuItem *item);
        int     Run(void);
        void    Null(void);
    private:

        void    _Render_Menu(void);
        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _ProcessEvent(Event &event);
        void    _Update(Time delta);

        void    _TriggerEntry(void);
        void    _StarItem(void);
        void    _StarMode(void);
        void    _DisplayNote(void);

        std::vector<MenuEntry *>    _executeLoop;
        std::queue<int>             _freeIndex;
        MenuFolder                  *_folder;
        MenuFolder                  *_starred;
        bool                        _isOpen;
        bool                        _starMode;

        int                         _selector;
        int                         _selectedTextSize;
        float                       _maxScrollOffset;
        float                       _scrollOffset;
        Clock                       _scrollClock;
        bool                        _reverseFlow;
        bool                        _pluginRun;
        IntVector                   _startLine;
        IntVector                   _endLine;

        Button<Menu, void>          _gameGuideBtn;
        CheckedButton<Menu, void>   _showStarredBtn;
        Button<Menu, void>          _toolsBtn;
        Button<Menu, void>          _hidMapperBtn;
        Button<Menu, void>          _searchBtn;

        ToggleButton<Menu, void>      _AddFavoriteBtn;
        ToggleButton<Menu, void>      _InfoBtn;

        TextBox                     *_noteTB;
    };
}

#endif
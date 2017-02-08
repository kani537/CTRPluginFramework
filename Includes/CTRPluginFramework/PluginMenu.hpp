#ifndef CTRPLUGINFRAMEWORK_PLUGINMENU_HPP
#define CTRPLUGINFRAMEWORK_PLUGINMENU_HPP

#include "Vector.hpp"
#include "Button.hpp"
#include "ToggleButton.hpp"
#include "CheckedButton.hpp"
#include "Clock.hpp"
#include "CTRPluginFramework/Graphics/TextBox.hpp"
#include "CTRPluginFramework/GuideReader.hpp"

#include <queue>

namespace CTRPluginFramework
{
    class MenuEntry;
    class MenuItem;
    class MenuFolder;
    class Event;
    class Time;
    class Clock;
    class PluginMenu
    {

    public:

        PluginMenu(std::string name = "Cheats", std::string note = "");
        ~PluginMenu(void);

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

        Button<PluginMenu, void>          _gameGuideBtn;
        CheckedButton<PluginMenu, void>   _showStarredBtn;
        Button<PluginMenu, void>          _toolsBtn;
        Button<PluginMenu, void>          _hidMapperBtn;
        Button<PluginMenu, void>          _searchBtn;

        ToggleButton<PluginMenu, void>      _AddFavoriteBtn;
        ToggleButton<PluginMenu, void>      _InfoBtn;

        TextBox                     *_noteTB;
        GuideReader                 _guide;
        void                        _TriggerGuide(void);
        int                         _mode;
    };
}

#endif
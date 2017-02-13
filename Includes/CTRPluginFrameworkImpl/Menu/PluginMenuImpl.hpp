#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUIMPL_HPP

#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/GuideReader.hpp"

#include <queue>

namespace CTRPluginFramework
{
    class PluginMenuImpl
    {

    public:

        PluginMenuImpl(std::string name = "Cheats", std::string note = "");
        ~PluginMenuImpl(void);

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

        std::vector<MenuEntryImpl *>    _executeLoop;
        std::queue<int>             _freeIndex;
        MenuFolderImpl             *_folder;
        MenuFolderImpl             *_starred;
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

        Button<PluginMenuImpl, void>          _gameGuideBtn;
        CheckedButton<PluginMenuImpl, void>   _showStarredBtn;
        Button<PluginMenuImpl, void>          _toolsBtn;
        Button<PluginMenuImpl, void>          _hidMapperBtn;
        Button<PluginMenuImpl, void>          _searchBtn;

        ToggleButton<PluginMenuImpl, void>      _AddFavoriteBtn;
        ToggleButton<PluginMenuImpl, void>      _InfoBtn;

        TextBox                     *_noteTB;
        GuideReader                 _guide;
        void                        _TriggerGuide(void);
        int                         _mode;
    };
}

#endif
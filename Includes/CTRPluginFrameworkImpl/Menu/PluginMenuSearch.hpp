#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUSEARCH_HPP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUSEARCH_HPP

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"
#include "CTRPluginFrameworkImpl/Search/Search32.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenu_SearchMenu.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuFreeCheats.hpp"


#include <vector>
#include <list>
#include "CTRPluginFrameworkImpl/Graphics/UIContainer.hpp"

namespace CTRPluginFramework
{
    class PluginMenuSearch
    {
        using EventList = std::vector<Event>;
    public:
        PluginMenuSearch(HexEditor &hexEditor, FreeCheats &freeCheats);
        ~PluginMenuSearch(){}

        // Return true if the Close Button is pressed, else false
        bool    operator()(EventList &eventList, Time &delta);
        void    GetRegionsList(std::vector<Region> &list);
        void    RestoreSearchState(void);
    private:

        void    _ProcessEvent(Event &event);
        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _Update(Time delta);

        void    _ListRegion(void);
        void    _PopulateSearchType(bool isFirstSearch);

        void    _searchBtn_OnClick(void);
        void    _resetBtn_OnClick(void);
        void    _undoBtn_OnClick(void);
        void    _cancelBtn_OnClick(void);

        void    _ShowProgressWindow(void) const;

        // Members
        std::vector<Region>                 _regionsList;
        std::list<Search *>                 _searchHistory;
        Search                              *_currentSearch;
        HexEditor                           &_hexEditor;

        bool                                _inSearch;
        bool                                _inEditor;
        bool                                _firstRegionInit;
        bool                                _waitForUser;
		bool								_hexInput;
        int                                 _step;

        // UIComponent
        SearchMenu                          _searchMenu;
        ComboBox                            _memoryRegions;
        ComboBox                            _searchSize; // Variable type
        ComboBox                            _searchType; // Unknown / Exact
        ComboBox                            _compareType; // Compare

        NumericTextBox                      _startRangeTextBox;
        NumericTextBox                      _endRangeTextBox;
        NumericTextBox                      _valueTextBox;

        // Buttons
        //IconButton<PluginMenuSearch, void>          _closeBtn;

        Button<PluginMenuSearch, void>              _searchBtn;
        Button<PluginMenuSearch, void>              _cancelBtn;
        Button<PluginMenuSearch, void>              _undoBtn;
        Button<PluginMenuSearch, void>              _resetBtn;

        CheckedButton<PluginMenuSearch, void>       _hexBtn;

        UIContainer                         _uiContainer;
    };
}

#endif
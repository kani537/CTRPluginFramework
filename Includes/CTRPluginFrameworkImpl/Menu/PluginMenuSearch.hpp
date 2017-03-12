#ifndef CTRPLUGINFRAMEWORKIMPL_PLUGINMENUSEARCH_HPP
#define CTRPLUGINFRAMEWORKIMPL_PLUGINMENUSEARCH_HPP

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearchStructs.hpp"

#include <vector>
#include <list>

namespace CTRPluginFramework
{
    class PluginMenuSearch
    {
        using EventList = std::vector<Event>;
    public:
        PluginMenuSearch();
        ~PluginMenuSearch(){}

        // Return true if the Close Button is pressed, else false
        bool    operator()(EventList &eventList, Time &delta);
    private:

        void    _ProcessEvent(Event &event);
        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _Update(Time delta);

        void    _DoSearch(void);
        void    _ShowProgressWindow(void);



        // Members
        std::list<SearchBase *>             _searchHistory;
        SearchBase                          *_currentSearch;

        // UIComponent
        ComboBox                            _memoryRegions;
        ComboBox                            _searchSize; // Variable type
        ComboBox                            _searchType; // Unknown / Exact
        ComboBox                            _compareType; // Compare
 
        NumericTextBox                      _alignmentTextBox;
        NumericTextBox                      _valueTextBox;

        // Buttons        
        IconButton<PluginMenuSearch, void>          _closeBtn;

        Button<PluginMenuSearch, void>              _searchBtn;
        Button<PluginMenuSearch, void>              _undoBtn;
        Button<PluginMenuSearch, void>              _resetBtn;

    };
}

#endif
#ifndef CTRPLUGINFRAMEWORKIMPL_SearchMenu_HPP
#define CTRPLUGINFRAMEWORKIMPL_SearchMenu_HPP


#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"
#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearchStructs.hpp"

#include <string>
#include <vector>

namespace CTRPluginFramework
{
    class SearchMenu
    {
        using EventList = std::vector<Event>;
    public:

        SearchMenu(SearchBase* &curSearch);
        ~SearchMenu(){};

        bool    ProcessEvent(EventList &eventList, Time &delta);
        void    Draw(void);
        void    Update(void);

    private:

        SearchBase*                 &_currentSearch;
        std::vector<std::string>    _resultsAddress;
        std::vector<std::string>    _resultsNewValue;
        std::vector<std::string>    _resultsOldValue;

        u32                         _index;


    };
}

#endif
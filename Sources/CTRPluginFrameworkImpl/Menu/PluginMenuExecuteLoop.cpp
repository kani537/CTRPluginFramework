#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"

#include <queue>

namespace CTRPluginFramework
{
    PluginMenuExecuteLoop::PluginMenuExecuteLoop(void)
    {

    }

    void    PluginMenuExecuteLoop::Add(MenuEntryImpl *entry)
    {
        _executeLoop.push_back(entry);
    }

    void    PluginMenuExecuteLoop::Remove(MenuEntryImpl *entry)
    {
        _executeLoop.erase(entry->_executeIndex);
    }

    bool    operator()(void)
    {
        std::queue<int> toRemove;

        for (int i = 0; i < _executeLoop.size(); i++)
        {
            MenuEntryImpl *entry = _executeLoop[i];
            if (entry)
            {
                if (entry->_Execute())
                {
                    toRemove.push(i);                   
                }
            }
        }
        while (!toRemove.empty())
        {
            int i = toRemove.front;
            toRemove.pop();
            _executeLoop.erase(i);
        }          
    }
    return (false);
}

#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"

#include <queue>

namespace CTRPluginFramework
{
    PluginMenuExecuteLoop *PluginMenuExecuteLoop::_firstInstance = nullptr;

    PluginMenuExecuteLoop::PluginMenuExecuteLoop(void)
    {
        _firstInstance = this;
    }

    void    PluginMenuExecuteLoop::Add(MenuEntryImpl *entry)
    {
        entry->_executeIndex = _firstInstance->_executeLoop.size();
        _firstInstance->_executeLoop.push_back(entry);

        if (entry->_flags.isRadio)
        {
            int id = entry->_radioId;
            std::vector<MenuEntryImpl *> &vector = _firstInstance->_executeLoop;

            if (vector.size() - 1 > 0)
            {
                for (int i = 0; i < vector.size() - 1; i++)
                {
                    MenuEntryImpl *v = vector[i];

                    if (v == entry)
                        continue;
                    if (v->_flags.isRadio && v->_radioId == id)
                    {
                        if (v->_flags.state)
                        {
                            v->_TriggerState();
                        }
                    }
                }                
            }

        }
    }

    void    PluginMenuExecuteLoop::Remove(MenuEntryImpl *entry)
    {
        _firstInstance->_executeLoop.erase(_firstInstance->_executeLoop.begin() + (int)entry->_executeIndex);
        entry->_executeIndex = -1;
    }

    bool    PluginMenuExecuteLoop::operator()(void)
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
            int i = toRemove.front();
            toRemove.pop();
            _executeLoop.erase(_executeLoop.begin() + i);
        }
        return (false);
    }
}

#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"

#include <queue>

namespace CTRPluginFramework
{
    using ExecuteIterator = std::vector<MenuEntryImpl *>::iterator;

    PluginMenuExecuteLoop *PluginMenuExecuteLoop::_firstInstance = nullptr;

    PluginMenuExecuteLoop::PluginMenuExecuteLoop(void)
    {
        _firstInstance = this;
    }

    void    PluginMenuExecuteLoop::Add(MenuEntryImpl *entry)
    {
        if (_firstInstance == nullptr || entry == nullptr)
            return;

        std::vector<MenuEntryImpl*> &vector = _firstInstance->_executeLoop;
        std::queue<int>             &queue = _firstInstance->_availableIndex;

        int     id = entry->_radioId;
        bool    alreadyInHere = false;

        // If it's a radio entry
        if (entry->_flags.isRadio && id != -1 && !vector.empty())
        { 
            for (int i = 0; i < vector.size(); i++)
            {
                MenuEntryImpl *e = vector[i];

                if (e != nullptr)
                {
                    // Check that it's not the same entry that we try to add
                    if (e == entry)
                    {
                        alreadyInHere = true;
                        continue;
                    }

                    if (e->_flags.state && e->_flags.isRadio && e->_radioId == id)
                    {
                        // If the entry was just added to the execute loop
                        if (e->_flags.justChanged)
                            Remove(e);
                        else
                            e->_TriggerState();
                    }
                }
            }
        }

        // If it's a non radio entry, check that the entry isn't in the vector
        if (!entry->_flags.isRadio)
        {
            for (int i = 0; i < vector.size(); i++)
            {
                MenuEntryImpl *e = vector[i];

                if (e == entry)
                    alreadyInHere = true;
            }
        }

        // If the entry is already in the loop, exit
        if (alreadyInHere)
            return;

        // If queue is empty
        if (queue.empty())
        {
            entry->_executeIndex = vector.size();
            vector.push_back(entry);            
        }
        else
        {
            int i = queue.front();
            queue.pop();

            vector[i] = entry;
            entry->_executeIndex = i;
        }
    }


    void    PluginMenuExecuteLoop::Remove(MenuEntryImpl *entry)
    {
        if (_firstInstance == nullptr || entry == nullptr)
            return;

        if (_firstInstance->_executeLoop.empty())
            return;


        std::vector<MenuEntryImpl *>    &vector = _firstInstance->_executeLoop;        
        std::queue<int>                 &queue = _firstInstance->_availableIndex;

        int id = entry->_executeIndex;

        if (id == -1)
            return;

        entry->_executeIndex = -1;
        entry->_flags.state = false;
        entry->_flags.justChanged = false;
        vector[id] = nullptr;

        queue.push(id);
    }

    bool    PluginMenuExecuteLoop::operator()(void)
    {
        if (_executeLoop.empty())
            return (false);

        for (int i = 0; i < _executeLoop.size(); i++)
        {
            MenuEntryImpl *entry = _executeLoop[i];

            if (entry != nullptr)
            {
                if (entry->_Execute())
                {
                    Remove(entry);                 
                }
            }
        }

        return (false);
    }
}


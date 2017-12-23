#include "CTRPluginFrameworkImpl/Menu/PluginMenuExecuteLoop.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryFreeCheat.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/MenuEntryActionReplay.hpp"

#include <queue>

extern CTRPluginFramework::PluginMenuExecuteLoop *g_executerInstance;
namespace CTRPluginFramework
{
    using ExecuteIterator = std::vector<MenuEntryImpl *>::iterator;

    PluginMenuExecuteLoop *PluginMenuExecuteLoop::_firstInstance = nullptr;

    PluginMenuExecuteLoop::PluginMenuExecuteLoop(void)
    {
        _firstInstance = this;
        g_executerInstance = this;
    }

    void PluginMenuExecuteLoop::WriteEnabledCheatsToFile(Preferences::Header& header, File& file)
    {
        if (_firstInstance == nullptr)
            return;

        std::vector<u32>    uids;
        std::vector<MenuEntryImpl *>  &items = _firstInstance->_executeLoop;
        u32     count = items.size();
        u32     index = 0;
        u32     written = 0;
        u64     offset = file.Tell();

        while (count)
        {
            uids.clear();
            u32 nb = count > 1000 ? 1000 : count;

            for (; index < items.size(); ++index)
            {
                MenuEntryImpl *e = items[index];

                if (e != nullptr && e->IsEntry() && e->IsActivated())
                    uids.push_back(e->Uid);
            }

            if (file.Write(uids.data(), sizeof(u32) * uids.size()) != 0)
                goto error;

            written += uids.size();
            count -= nb;
        }
        header.enabledCheatsCount = written;
        header.enabledCheatsOffset = offset;

        error:
            return;
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

    void    ActionReplay_FetchList(void);
    bool    PluginMenuExecuteLoop::operator()(void)
    {
        static bool isBusy = false;

        if (isBusy)
            return false;

        if (_executeLoop.empty())
            return false;

        isBusy = true;

      //  ActionReplay_FetchList();
        for (int i = 0; i < _executeLoop.size(); i++)
        {
            MenuEntryImpl *entry = _executeLoop[i];

            if (entry != nullptr)
            {
                // Execute MenuEntryImpl
                if (entry->IsEntry() && entry->_Execute())
                {
                    Remove(entry);
                }
                // Execute FreeCheat
                else if (entry->IsFreeCheat())
                {
                    MenuEntryFreeCheat *fc = reinterpret_cast<MenuEntryFreeCheat *>(entry);

                    if (fc->Func != nullptr)
                        fc->Func(fc);
                }
                else if (entry->_type == ActionReplay)
                {
                    MenuEntryActionReplay *ar = reinterpret_cast<MenuEntryActionReplay *>(entry);

                    if (ar->GameFunc != nullptr)
                        ar->GameFunc((MenuEntry *)ar); ///< cast only to silence a warning
                }
            }
        }

        isBusy = false;
        return (false);
    }
}

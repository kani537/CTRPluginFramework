#include "MenuEntry.hpp"

namespace CTRPluginFramework
{
    MenuEntry::MenuEntry(std::string name, std::string note) : MenuItem(MenuType::Entry)
    {
        this->name = name;
        this->note = note;
        this->GameFunc = nullptr;
        this->MenuFunc = nullptr;
        this->_arg = nullptr;
    }

    MenuEntry::MenuEntry(std::string name, FuncPointer func, std::string note) : MenuItem(MenuType::Entry)
    {
        this->name = name;
        this->note = note;
        this->GameFunc = func;
        this->MenuFunc = nullptr;
        this->_arg = nullptr;
    }

    MenuEntry::~MenuEntry() {}

    void    MenuEntry::Disable(void)
    {
        _flags.state = 0;
    }

    void    MenuEntry::SetRadio(int id)
    {
        _flags.isRadio = 1;
        _radioId = id;
    }

    void    MenuEntry::SetArg(void *arg)
    {
        _arg = arg;
    }

    void    *MenuEntry::GetArg(void)
    {
        return (_arg);
    }

    bool    MenuEntry::WasJustActivated(void)
    {
        return (_flags.state && _flags.justChanged);
    }

    bool    MenuEntry::IsActivated(void)
    {
        return (_flags.state);
    }

    //##############################################################

    void    MenuEntry::_TriggerState(void)
    {
        if (_flags.state)
        {
            _flags.state = 0;
            _flags.justChanged = 0;
        }
        else
        {
            _flags.state = 1;
            _flags.justChanged = 1;
        }

        // TODO: Add the FuncPointer to the executable list
        // TODO: Parse executable list and disable radio entry with same ID
    }

    void    MenuEntry::_TriggerStar(void)
    {
        if (_flags.isStarred)
        {
            _flags.isStarred = 0;
        }
        else 
        {
            _flags.isStarred = 1;
        }

        // TODO: Add the object to the starred list
    }

    void    MenuEntry::_Execute(void)
    {
        GameFunc(this);
        _flags.justChanged = 0;

        if (!_flags.state)
        {
            // TODO: Remove the object to the executable list
        }
    }

}
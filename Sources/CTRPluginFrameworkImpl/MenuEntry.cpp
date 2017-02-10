#include "types.h"

#include "CTRPluginFramework/MenuItem.hpp"
#include "CTRPluginFramework/MenuEntry.hpp"

namespace CTRPluginFramework
{
    MenuEntry::MenuEntry(std::string name, std::string note) : MenuItem(MenuType::Entry)
    {
        this->name = name;
        this->note = note;
        this->GameFunc = nullptr;
        this->MenuFunc = nullptr;
        this->_arg = nullptr;
        this->_executeIndex = -1;
        this->_flags = {0};
        this->_radioId = -1;
    }

    MenuEntry::MenuEntry(std::string name, FuncPointer func, std::string note) : MenuItem(MenuType::Entry)
    {
        this->name = name;
        this->note = note;
        this->GameFunc = func;
        this->MenuFunc = nullptr;
        this->_arg = nullptr;
        this->_executeIndex = -1;        
        this->_flags = {0};        
        this->_radioId = -1;
    }

    MenuEntry::~MenuEntry() {}

    void    MenuEntry::Disable(void)
    {
        _flags.state = 0;
        _flags.justChanged = 1;
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

    bool    MenuEntry::_TriggerState(void)
    {
        if (_flags.state)
        {
            _flags.state = 0;
            _flags.justChanged = 0;
            return (false);
        }
        else
        {
            _flags.state = 1;
            _flags.justChanged = 1;
            return (true);
        }
    }

    bool    MenuEntry::_MustBeRemoved(void)
    {
        if (_flags.state)
            return (false);
        if (_flags.justChanged)
            return (false);
        return (true);
    }

    bool    MenuEntry::_Execute(void)
    {
        Flags fl = _flags;
        if (GameFunc != nullptr)
            GameFunc(this);
        
        if (_flags.state && _flags.justChanged)
            _flags.justChanged = 0;
        else if (!_flags.state && fl.justChanged && !fl.state)
            _flags.justChanged = 0;

        return (_MustBeRemoved());
    }

}
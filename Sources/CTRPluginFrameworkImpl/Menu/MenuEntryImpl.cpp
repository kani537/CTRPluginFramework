#include "types.h"

#include "CTRPluginFrameworkImpl/Menu/MenuItem.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"

namespace CTRPluginFramework
{
    MenuEntryImpl::MenuEntryImpl(std::string name, std::string note, MenuEntry *owner) : MenuItem(MenuType::Entry)
    {
        this->name = name;
        this->note = note;
        this->GameFunc = nullptr;
        this->MenuFunc = nullptr;
        this->_arg = nullptr;
        this->_executeIndex = -1;
        this->_flags = {0};
        this->_radioId = -1;
        this->_owner = owner;
    }

    MenuEntryImpl::MenuEntryImpl(std::string name, FuncPointer func, std::string note, MenuEntry *owner) : MenuItem(MenuType::Entry)
    {
        this->name = name;
        this->note = note;
        this->GameFunc = func;
        this->MenuFunc = nullptr;
        this->_arg = nullptr;
        this->_executeIndex = -1;        
        this->_flags = {0};        
        this->_radioId = -1;
        this->_owner = owner;
    }

    void    MenuEntryImpl::Disable(void)
    {
        _flags.state = 0;
        _flags.justChanged = 1;
    }

    void    MenuEntryImpl::SetRadio(int id)
    {
        _flags.isRadio = 1;
        _radioId = id;
    }

    void    MenuEntryImpl::SetArg(void *arg)
    {
        _arg = arg;
    }

    void    *MenuEntryImpl::GetArg(void)
    {
        return (_arg);
    }

    bool    MenuEntryImpl::WasJustActivated(void)
    {
        return (_flags.state && _flags.justChanged);
    }

    bool    MenuEntryImpl::IsActivated(void)
    {
        return (_flags.state);
    }

    //##############################################################

    bool    MenuEntryImpl::_TriggerState(void)
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

    bool    MenuEntryImpl::_MustBeRemoved(void)
    {
        if (_flags.state)
            return (false);
        if (_flags.justChanged)
            return (false);
        return (true);
    }

    bool    MenuEntryImpl::_Execute(void)
    {
        Flags fl = _flags;
        if (GameFunc != nullptr)
            GameFunc(_owner);
        
        if (_flags.state && _flags.justChanged)
            _flags.justChanged = 0;
        else if (!_flags.state && fl.justChanged && !fl.state)
            _flags.justChanged = 0;

        return (_MustBeRemoved());
    }

}
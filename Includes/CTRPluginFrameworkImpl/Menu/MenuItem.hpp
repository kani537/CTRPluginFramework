#ifndef CTRPLUGINFRAMEWORKIMPL_MENUITEMIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_MENUITEMIMPL_HPP

#include <string>

namespace CTRPluginFramework
{
    enum MenuType
    {
        Folder,
        Entry
    };

    class MenuItem
    {
    public:
        MenuItem(MenuType type) : _type(type) , _isStarred(false)
        {}

        std::string     name;
        std::string     note;

    private:
        friend class PluginMenuImpl;
        friend class Menu;

        bool        _IsStarred(void)
        {
            return (_isStarred);
        }

        bool        _TriggerStar(void)
        {
            _isStarred = !_isStarred;
            return (_isStarred);
        }
        MenuType    _type;
        bool        _isStarred;
    };
}

#endif
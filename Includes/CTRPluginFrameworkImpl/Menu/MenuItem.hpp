#ifndef CTRPLUGINFRAMEWORKIMPL_MENUITEMIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_MENUITEMIMPL_HPP

#include "types.h"
#include <string>

namespace CTRPluginFramework
{
    enum MenuType
    {
        Folder,
        Entry,
        EntryTools
    };

    class MenuFolderImpl;
    class MenuItem
    {
    public:
        MenuItem(MenuType type) :
        _type(type), _isStarred(false), _isVisible(true), _container(nullptr), _index(0), _uid(++_uidCounter)
        {
        }

        std::string     name;
        std::string     note;

        void    Hide(void);
        void    Show(void);
        bool    IsVisible(void) const
        {
            return (_isVisible);
        }

        bool    IsEntry(void) const
        {
            return (_type == Entry);
        }

    protected:
        friend class MenuFolderImpl;
        friend class PluginMenuImpl;
        friend class PluginMenuHome;
        friend class Menu;

        static void     _DisableFolder(MenuFolderImpl *folder);
        static void     _EnableFolder(MenuFolderImpl* folder);

        bool        _IsStarred(void) const
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
        bool        _isVisible;
        MenuItem    *_container; /* MenuFolderImpl */
        int         _index;
        const u32   _uid;

        static u32  _uidCounter;
    };
}

#endif
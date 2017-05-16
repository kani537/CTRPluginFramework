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

    class MenuFolderImpl;
    class MenuItem
    {
    public:
        MenuItem(MenuType type) :
        _type(type), _isStarred(false), _isVisible(true), _container(nullptr), _index(0)
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

    private:
        friend class MenuFolderImpl;
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
    };
}

#endif
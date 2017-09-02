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
        EntryTools,
        FreeCheat
    };

    class MenuEntryImpl;
    class MenuEntryTools;
    class MenuFolderImpl;
    class MenuItem
    {
    public:
        MenuItem(MenuType type) :
        _type(type), _isStarred(false),
        _isVisible(true),
        _hasNoteChanged(false),
        _container(nullptr), _index(0), Uid(++_uidCounter)
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

        bool    IsFolder(void) const
        {
            return (_type == Folder);
        }

        bool    IsEntryTools(void) const
        {
            return (_type == EntryTools);
        }

        bool    IsFreeCheat(void) const
        {
            return (_type == FreeCheat);
        }

        MenuEntryImpl &AsMenuEntryImpl(void)
        {
            return (*reinterpret_cast<MenuEntryImpl *>(this));
        }

        MenuEntryTools &AsMenuEntryTools(void)
        {
            return (*reinterpret_cast<MenuEntryTools *>(this));
        }

        MenuFolderImpl &AsMenuFolderImpl(void)
        {
            return (*reinterpret_cast<MenuFolderImpl *>(this));
        }

        virtual std::string &GetNote(void)
        {
            return (note);
        }

        void    NoteChanged(void);
        bool    HasNoteChanged(void) const;
        void    HandledNoteChanges(void);

        const u32   Uid;
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
        bool        _hasNoteChanged;
        MenuItem    *_container; /* MenuFolderImpl */
        int         _index;
        

        static u32  _uidCounter;
    };
}

#endif
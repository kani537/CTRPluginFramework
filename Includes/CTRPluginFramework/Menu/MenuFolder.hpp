#ifndef CTRPLUGINFRAMEWORK_MENUFOLDER_HPP
#define CTRPLUGINFRAMEWORK_MENUFOLDER_HPP

#include <string>
#include <memory>

namespace CTRPluginFramework
{
    class MenuFolder
    {

    public:
        MenuFolder(std::string name, std::string note = "");
        ~MenuFolder(){};

        void    Append(MenuEntry *item);
        void    Append(MenuFolder *item);
        u32     ItemsCount(void);

    private:
        class MenuFolderImpl;
        std::unique_ptr<MenuFolderImpl> _item;
    };
}

#endif

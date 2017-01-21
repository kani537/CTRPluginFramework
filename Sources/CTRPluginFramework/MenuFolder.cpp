#include "MenuFolder.hpp"

namespace CTRPluginFramework
{
    MenuFolder::MenuFolder(std::string name, std::string note) :
    MenuItem(MenuType::Folder)
    {
        this->name = name;
        this->infos = note;
    }

    bool    MenuFolder::Append(MenuItem)
}
#ifndef CTRPLUGINFRAMEWORKIMPL_GUIDEREADER_HPP
#define CTRPLUGINFRAMEWORKIMPL_GUIDEREADER_HPP

#include "types.h"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuItem.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuFolderImpl.hpp"
#include "CTRPluginFramework/System/Directory.hpp"
#include "CTRPluginFramework/System/File.hpp"
#include "CTRPluginFrameworkImpl/Graphics/TextBox.hpp"

namespace CTRPluginFramework
{
    class GuideReader
    {
    public:
        GuideReader(void);
        ~GuideReader(void){};
        bool    Draw(void);
        bool    ProcessEvent(Event &event);

        void    Open(void);
        void    Close(void);
        bool    IsOpen(void);

    private:
        bool            _isOpen;
        Menu            _menu;
        TextBox         *_guideTB;
        std::string     _text;
        MenuEntryImpl   *_last;
    };
}

#endif
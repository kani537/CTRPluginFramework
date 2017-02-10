#ifndef CTRPLUGINFRAMEWORK_GUIDEREADER_HPP
#define CTRPLUGINFRAMEWORK_GUIDEREADER_HPP

#include "types.h"
#include "CTRPluginFramework/Menu.hpp"
#include "CTRPluginFramework/MenuItem.hpp"
#include "CTRPluginFramework/MenuEntry.hpp"
#include "CTRPluginFramework/MenuFolder.hpp"
#include "CTRPluginFramework/Directory.hpp"
#include "CTRPluginFramework/File.hpp"
#include "CTRPluginFramework/Graphics/TextBox.hpp"

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
        MenuEntry       *_last;
    };
}

#endif
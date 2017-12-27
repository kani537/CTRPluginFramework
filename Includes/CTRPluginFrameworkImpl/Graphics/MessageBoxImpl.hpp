#ifndef CTRPLUGINFRAMEWORKIMPL_MESSAGEBOXIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_MESSAGEBOXIMPL_HPP

#include <string>
#include "CTRPluginFramework/System/Rect.hpp"
#include "CTRPluginFrameworkImpl/System/Events.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "CTRPluginFramework/Graphics/Color.hpp"
#include "TextBox.hpp"

namespace CTRPluginFramework
{

    class MessageBoxImpl
    {
    public:
        MessageBoxImpl(const std::string &title, const std::string &message, DialogType dtype = DialogType::DialogOk);
        MessageBoxImpl(const std::string &message, DialogType dtype = DialogType::DialogOk);
        ~MessageBoxImpl() {};

        /*
        ** Return true if user pressed OK, false if user choose Cancel
        ** Return always true if type is DialogOkOnly
        */
        bool    operator()(void);
    private:

        void    _ProcessEvent(Event &event);
        void    _Draw(void);

        Color           _titleColor;
        std::string     _title;
        std::string     _message;
        DialogType      _dialogType;
        TextBox         _textbox;
        bool            _exit;
        int             _cursor;
        IntRect         _box;

    };
}

#endif
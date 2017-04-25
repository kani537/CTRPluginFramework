#ifndef CTRPLUGINFRAMEWORKIMPL_MESSAGEBOXIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_MESSAGEBOXIMPL_HPP

#include <string>
#include "Rect.hpp"
#include "CTRPluginFrameworkImpl/System/Events.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"

namespace CTRPluginFramework
{

    class MessageBoxImpl
    {
    public:
        MessageBoxImpl(std::string message, DialogType dtype = DialogType::DialogOk);
        ~MessageBoxImpl() {};

        /*
        ** Return true if user pressed OK, false if user choose Cancel
        ** Return always true if type is DialogOkOnly
        */
        bool    operator()(void);
    private:

        void    _ProcessEvent(Event &event);
        void    _Draw(void);

        std::string     _message;
        DialogType      _dialogType;
        bool            _exit;
        int             _cursor;
        IntRect         _box;

    };
}

#endif
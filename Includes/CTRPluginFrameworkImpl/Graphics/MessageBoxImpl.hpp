#ifndef CTRPLUGINFRAMEWORKIMPL_MESSAGEBOXIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_MESSAGEBOXIMPL_HPP

#include <string>

namespace CTRPluginFramework
{
    enum class DialogType
    {
        DialogOkOnly,
        DialogOkCancel
    };
    class MessageBoxImpl
    {
    public:
        MessageBoxImpl(std::string message, DialogType dtype = DialogType::DialogOkOnly);
        ~MessageBoxImpl() {};

        /*
        ** Return true if user pressed OK, false if user choose Cancel
        ** Return always true if type is DialogOkOnly
        */
        bool    operator()(void);
    private:
        std::string     _message;
        DialogType      _dialogType;

    };
}

#endif
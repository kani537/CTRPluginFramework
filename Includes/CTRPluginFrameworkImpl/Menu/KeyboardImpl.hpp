#ifndef CTRPLUGINFRAMEWORKIMPL_KEYBOARD_HPP
#define CTRPLUGINFRAMEWORKIMPL_KEYBOARD_HPP

#include "CTRPluginFramework/System/Touch.hpp"
#include "CTRPluginFrameworkImpl/Graphics/TouchKey.hpp"

#include <vector>

namespace CTRPluginFramework
{
    class KeyboardImpl
    {
        using   CompareCallback = bool (*)(const void *, std::string&);
        using   ConvertCallback = void *(*)(std::string&);
        using   KeyIter  = std::vector<TouchKey>::iterator;
    public:

        enum Layout
        {
            QWERTY,
            DECIMAL,
            HEXADECIMAL
        };

        KeyboardImpl(void);
        ~KeyboardImpl(void);

        void    SetLayout(Layout layout);

        int     Run(void);
    private:

        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _ProcessEvent(Event &event);
        void    _Update(void);

        // Keyboard layout constructor
        void    _Qwerty(void);
        void    _DigitKeyboard(void);
        void    _Decimal(void);
        void    _Hexadecimal(void);

        bool    _CheckKeys(void); //<- Return if input have changed
        bool    _CheckInput(void); //<- Call compare callback, return true if the input is valid

        std::string             _text;
        std::string             _error;
        std::string             _userInput;

        bool                    _isOpen;
        bool                    _askForExit;
        bool                    _errorMessage;
        Layout                  _layout;

        CompareCallback         _compare;
        ConvertCallback         _convert;

        std:vector<TouchKey>    _keys;

    };
}

#endif
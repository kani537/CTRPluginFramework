#ifndef CTRPLUGINFRAMEWORKIMPL_KEYBOARD_HPP
#define CTRPLUGINFRAMEWORKIMPL_KEYBOARD_HPP

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Graphics/TouchKey.hpp"
#include "CTRPluginFrameworkImpl/System.hpp"

#include <vector>
#include <string>

namespace CTRPluginFramework
{
    enum Layout
    {
        QWERTY,
        DECIMAL,
        HEXADECIMAL
    };

    class KeyboardImpl
    {
        using   CompareCallback = bool (*)(const void *, std::string&);
        using   ConvertCallback = void *(*)(std::string&, bool);
        using   KeyIter  = std::vector<TouchKey>::iterator;
    public:

        KeyboardImpl(std::string text = "");
        ~KeyboardImpl(void);

        void        SetLayout(Layout layout);
        void        SetHexadecimal(bool isHex);
        void        SetMaxInput(int max);
        std::string &GetInput(void);
        void        SetConvertCallback(ConvertCallback callback);
        void        SetCompareCallback(CompareCallback callback);

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
        bool                    _userAbort;
        bool                    _isHex;
        int                     _max;
        Layout                  _layout;

        CompareCallback         _compare;
        ConvertCallback         _convert;

        std::vector<TouchKey>    _keys;

    };
}

#endif
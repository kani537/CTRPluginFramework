#ifndef CTRPLUGINFRAMEWORKIMPL_KEYBOARD_HPP
#define CTRPLUGINFRAMEWORKIMPL_KEYBOARD_HPP

#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFrameworkImpl/Graphics/TouchKey.hpp"
#include "CTRPluginFrameworkImpl/Graphics/TouchKeyString.hpp"
#include "CTRPluginFramework/Menu/Keyboard.hpp"
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

    class Keyboard;
    class KeyboardImpl
    {
        using   CompareCallback = bool (*)(const void *, std::string&);
        using   ConvertCallback = void *(*)(std::string&, bool);
        using   OnInputChangeCallback = void(*)(Keyboard&, InputChangeEvent&);
        using   KeyIter  = std::vector<TouchKey>::iterator;
        using   KeyStringIter  = std::vector<TouchKeyString>::iterator;
    public:

        KeyboardImpl(const std::string &text = "");
        explicit KeyboardImpl(Keyboard *kb, const std::string &text = "");
        ~KeyboardImpl(void);

        void        SetLayout(Layout layout);
        void        SetHexadecimal(bool isHex);
        bool        IsHexadecimal(void) const;
        void        SetMaxInput(int max);
        void        CanAbort(bool canAbort);
        void        CanChangeLayout(bool canChange);
        std::string &GetInput(void);
        std::string &GetMessage(void);
        void        SetError(std::string &error);

        void        SetConvertCallback(ConvertCallback callback);
        void        SetCompareCallback(CompareCallback callback);
        void        OnInputChange(OnInputChangeCallback callback);
        void        Populate(const std::vector<std::string> &input);

        int         Run(void);
        void        Close(void);
        bool        operator()(int &out);

        bool        DisplayTopScreen;
    private:
        friend class HexEditor;
        friend class ARCodeEditor;

        void    _RenderTop(void);
        void    _RenderBottom(void);
        void    _ProcessEvent(Event &event);
        void    _Update(float delta);

        // Keyboard layout constructor
        void    _Qwerty(void);
        void    _QwertyLowCase(void);
        void    _QwertyUpCase(void);
        void    _QwertySymbols(void);
        void    _QwertyNintendo(void);
        static void    _DigitKeyboard(std::vector<TouchKey> &keys);
        void    _Decimal(void);
        void    _Hexadecimal(void);

        void    _ScrollUp(void);
        void    _ScrollDown(void);
        void    _UpdateScrollInfos(const char *cstr, float strWidth);
        bool    _CheckKeys(void); //<- Return if input have changed
        bool    _CheckInput(void); //<- Call compare callback, return true if the input is valid

        bool    _CheckButtons(int &ret); //<- for string button

        Keyboard                *_owner;

        std::string             _text;
        std::string             _error;
        std::string             _userInput;

        bool                    _canChangeLayout;
        bool                    _canAbort;
        bool                    _isOpen;
        bool                    _askForExit;
        bool                    _errorMessage;
        bool                    _userAbort;
        bool                    _isHex;
        bool                    _mustRelease;
        bool                    _useCaps;
        bool                    _useSymbols;
        bool                    _useNintendo;
        float                   _offset;
        int                     _max;
        u8                      _symbolsPage;
        u8                      _nintendoPage;
        Layout                  _layout;
        Clock                   _blinkingClock;
        int                     _cursorPositionInString;
        int                     _cursorPositionOnScreen;
        bool                    _showCursor;

        CompareCallback         _compare;
        ConvertCallback         _convert;
        OnInputChangeCallback   _onInputChange;
        InputChangeEvent        _inputChangeEvent;
        std::vector<TouchKey>    *_keys;

        static std::vector<TouchKey>    _DecimalKeys;
        static std::vector<TouchKey>    _HexaDecimalKeys;
        static std::vector<TouchKey>    _QwertyKeys;

        // Custom keyboard stuff
        bool                    _customKeyboard;
        bool                    _displayScrollbar;
        int                     _currentPosition;
        u32                     _scrollbarSize;
        u32                     _scrollCursorSize;
        float                   _scrollSize;
        float                   _scrollPosition;
        float                   _scrollPadding;
        float                   _scrollJump;
        float                   _inertialVelocity;
        float                   _scrollStart;
        float                   _scrollEnd;
        IntVector               _lastTouch;
        Clock                   _touchTimer;

        std::vector<TouchKeyString *> _strKeys;


    };
}

#endif
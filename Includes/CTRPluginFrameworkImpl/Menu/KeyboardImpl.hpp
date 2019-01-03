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
        void        Clear(void);

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
        void    _UpdateScrollInfos(void);
        bool    _CheckKeys(void); //<- Return if input have changed
        bool    _CheckInput(void); //<- Call compare callback, return true if the input is valid

        bool    _CheckButtons(int &ret); //<- for string button

        Keyboard                *_owner{nullptr};

        std::string             _text;
        std::string             _error;
        std::string             _userInput;

        bool                    _canChangeLayout{false};
        bool                    _canAbort{true};
        bool                    _isOpen{false};
        bool                    _askForExit{false};
        bool                    _errorMessage{false};
        bool                    _userAbort{false};
        bool                    _isHex{true};
        bool                    _mustRelease{false};
        bool                    _useCaps{false};
        bool                    _useSymbols{false};
        bool                    _useNintendo{false};
        float                   _offset{0.f};
        int                     _max{0};
        u8                      _symbolsPage{0};
        u8                      _nintendoPage{0};
        Layout                  _layout{HEXADECIMAL};
        Clock                   _blinkingClock;
        int                     _cursorPositionInString{0};
        int                     _cursorPositionOnScreen{0};
        bool                    _showCursor{true};

        CompareCallback         _compare{nullptr};
        ConvertCallback         _convert{nullptr};
        OnInputChangeCallback   _onInputChange{nullptr};
        InputChangeEvent        _inputChangeEvent{};
        std::vector<TouchKey>    *_keys{nullptr};

        static std::vector<TouchKey>    _DecimalKeys;
        static std::vector<TouchKey>    _HexaDecimalKeys;
        static std::vector<TouchKey>    _QwertyKeys;

        // Custom keyboard stuff
        bool                    _customKeyboard{false};
        bool                    _displayScrollbar{false};
        int                     _currentPosition{0};
        u32                     _scrollbarSize{0};
        u32                     _scrollCursorSize{0};
        float                   _scrollSize{0.f};
        float                   _scrollPosition{0.f};
        float                   _scrollPadding{0.f};
        float                   _scrollJump{0.f};
        float                   _inertialVelocity{0.f};
        float                   _scrollStart{0.f};
        float                   _scrollEnd{0.f};
        IntVector               _lastTouch;
        Clock                   _touchTimer;

        std::vector<TouchKeyString *> _strKeys;
    };
}

#endif

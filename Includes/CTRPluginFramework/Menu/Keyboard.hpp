#ifndef CTRPLUGINFRAMEWORK_KEYBOARD_HPP
#define CTRPLUGINFRAMEWORK_KEYBOARD_HPP

#include "types.h"
#include <string>
#include <vector>
#include <memory>

namespace CTRPluginFramework
{
    class KeyboardImpl;
    class Keyboard
    {
        using   CompareCallback = bool (*)(const void *, std::string&);
        using   OnInputChangeCallback = void(*)(Keyboard&);
    public:
        Keyboard(std::string text = "");
        ~Keyboard(void);

        /*
        ** Set if the user can abort the keybord by pressing B
        ** canAbort: true / false => whether the user can press B to close the keyboard
        ** and abort the current operation
        ************************************************/
        void    CanAbort(bool canAbort);

        /*
        ** Define if the input must be hexadecimal or not
        ** Have no effect for float, double, string
        **************************************************/
        void    IsHexadecimal(bool isHex);

        /*
        ** Define a callback to check the input
        ** Return value: true if the value is valid, false otherwise
        ***************************************************************/
        void    SetCompareCallback(CompareCallback callback);

        /*
        ** Define a callback that will be called when the user change the input
        ** Note that CompareCallback is called before OnInputChange if it's set
        ***************************************************************/
        void    OnInputChange(OnInputChangeCallback callback);

        /*
        ** Set the error flag and an error message
        ** error: the message to be displayed as an error
        ************************************************/
        void    SetError(std::string error);

        /*
        ** Populate a keyboard with strings
        ************************************************/
        void    Populate(std::vector<std::string> &input);

        /*
        ** Open(void)
        ** Only usable with a keyboard that you populated with strings
        ** Return value: 
        **    -1 : user abort / not populated
        **    >= 0 : index of the user choice in the vector
        ************************************************/
        int     Open(void);
        /*
        ** Open the keyboard and wait for user input
        ** Return value: 0 = success, -1 = user abort
        ************************************************/
        int     Open(u8 &output);
        int     Open(u8 &output, u8 start);
        int     Open(u16 &output);
        int     Open(u16 &output, u16 start);
        int     Open(u32 &output);
        int     Open(u32 &output, u32 start);
        int     Open(u64 &output);
        int     Open(u64 &output, u64 start);

        int     Open(float &output);
        int     Open(float &output, float start);
        int     Open(double &output);
        int     Open(double &output, double start);

        int     Open(std::string &output);
        int     Open(std::string &output, std::string start);

        /*
        ** Forcefully close the keyboard without any regard to the error flag
        ** (This can only be called from an OnInputChange callback)
        ************************************************/
        void    Close(void);

        /*
        ** Return a reference the Keyboard's input string
        ************************************************/
        std::string     &GetInput(void);

        /*
        ** Return a reference the top screen's string
        ************************************************/
        std::string     &GetMessage(void);        
        
        /*
        ** This property define if the top screen must be displayed or not
        ** If not, errors messages can't be displayed
        ************************************************/
        bool    DisplayTopScreen;

    private:
        std::unique_ptr<KeyboardImpl>   _keyboard;
        bool                            _hexadecimal;
        bool                            _isPopulated;
    };
}

#endif
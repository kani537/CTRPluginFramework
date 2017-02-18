#include "CTRPluginFrameworkImpl/Menu/Keyboard.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"

namespace CTRPluginFramework
{
    #define USER_VALID  0
    #define USER_ABORT  -1
    #define KEY_ENTER 0xA
    #define KEY_BACKSPACE 0x8

    KeyboardImpl::KeyboardImpl(void)
    {
        _text = "";
        _error = "";
        _userInput = "";

        _isOpen = false;
        _errorMessage = false;
        _askForExit = false;
        _layout = HEXADECIMAL;
    }

    KeyboardImpl::~KeyboardImpl(void)
    {

    }

    void    KeyboardImpl::SetLayout(Layout layout)
    {
        _layout = layout;
    }

    int     KeyboardImpl::Run(void)
    {
        _isOpen = true;

        bool    mustRelease = false;
        // Check if Process is paused
        if (!ProcessImpl::IsPaused())
        {
            mustRelease  = true;
            ProcessImpl::Pause();
        }

        int                 ret = -2;
        Event               event;
        EventManager        manager;

        // Construct keyboard
        if (_layout == QWERTY) { ret = -1; goto exit; }//<-- Currently unsupported
        else if (_layout == DECIMAL) _Decimal();
        else if (_layout == HEXADECIMAL) _Hexadecimal();

        // Loop until exit
        while (_isOpen)
        {
            while (manager.PollEvent(event))
            {
                _ProcessEvent(event);                
            }

            // Update current keys
            _Update();

            Renderer::StartFrame();
            // Render Top Screen
            _RenderTop();
            // Render Bottom Screen
            _RenderBottom();
            Renderer::EndFrame();

            // Check keys
            bool inputChanged = _CheckKeys();

            if (_errorMessage && inputChanged)
                _errorMessage = false;

            if (inputChanged && _compare != nullptr)
                _errorMessage = !_CheckInput();

            if (_askForExit && !_errorMessage)
                _isOpen = false;
            else if (_askForExit && _errorMessage)
                _askForExit = false;
        }

    exit:
        if (mustRelease)
            ProcessImpl::Play();
        return (ret);
    }

    void    KeyboardImpl::_RenderBottom(void)
    {
        static IntRect  background(20, 20, 280, 200);
    }

    void    KeyboardImpl::_Qwerty(void)
    {
        // TODO
        return;
    }

    /*
    ** _keys:
    **
    ** [0] = 'A'
    ** [1] = 'B'
    ** [2] = 'C'
    ** [3] = 'D'
    ** [4] = 'E'
    ** [5] = 'F'
    ** [6] = '1'
    ** [7] = '2'
    ** [8] = '3'
    ** [9] = '4'
    ** [10] = '5'
    ** [11] = '6'
    ** [12] = '7'
    ** [13] = '8'
    ** [14] = '9'
    ** [15] = KEY_BACKSPACE
    ** [16] = KEY_ENTER
    ** [17] = '.'
    ** [18] = '0'
    *************************/

    void    KeyboardImpl::_DigitKeyboard(void)
    {
        //Start
        IntRect pos(20, 36, 46, 46);

        // A - F
        char c = 'A';
        for (int i = 0; i < 6; i++, c++)
        {
            _keys.push_back(TouchKey(c, pos));
            pos.leftTop.x += 46;

            if (i % 2)
            {
                pos.leftTop.x = 20;
                pos.leftTop.y += 46;
            }
        }

        pos.leftTop.y = 36;
        pos.leftTop.x = 112;

        // 1 - 9
        c = '1';
        for (int i = 0; i < 9; i++, c++)
        {
            _keys.push_back(TouchKey(c, pos));
            pos.leftTop.x += 46;

            if (i % 3 == 2)
            {
                pos.leftTop.x = 112;
                pos.leftTop.y += 46;
            }
        }

        // Special keys
        pos.leftTop.y = 36;
        pos.leftTop.x = 250;

        // Clear key
        _keys.push_back(TouchKey(KEY_BACKSPACE, Icon::DrawClearSymbol, pos));
        pos.leftTop.y += 46;

        // Enter key
        _keys.push_back(TouchKey(KEY_ENTER, Icon::DrawEnterKey, pos));
        pos.leftTop.y += 46;

        // Dot key
        _keys.push_back(TouchKey('.', pos));

        // 0 key
        pos.leftTop.y = 174;
        pos.leftTop.x = 112;
        pos.size.x = 138;
        _keys.push_back(TouchKey('0', pos));    
    }

    void    KeyboardImpl::_Decimal(void)
    {
        _DigitKeyboard();

        // Disable Hex keys
        KeyIter  iter = _keys.begin();
        KeyIter  end = std::advance(iter, 6);

        for (; iter != end; iter++)
            (*iter).Enable(false);
    }

    void    KeyboardImpl::_Hexadecimal(void)
    {
        _DigitKeyboard();

        // Disable dot key
        _keys.at(17).Enable(false);
    }

    bool    KeyboardImpl::_CheckKeys(void)
    {
        char ret = -1;

        for (int i = 0; i < _keys.size(); i++)
        {
            ret = _keys[i]();
            if (ret != -1)
            {
                if (ret == KEY_ENTER)
                {
                    _askForExit = true;
                    return (false);
                }
                else if (ret == KEY_BACKSPACE)
                {
                    _userInput.pop_back();
                    return (true);
                }
                else
                {
                    _userInput += ret;
                    return (true); 
                }                
            }
        }
        return (false);
    }

    bool    KeyboardImpl::_CheckInput(void)
    {
        if (_compare != nullptr && _convert != nullptr)
        {
            void *convertedInput = _convert(_userInput);
            return (_compare(convertedInput, _error));
        }
        // In case there's no callback, always consider input as valid
        return (true);
    }
}
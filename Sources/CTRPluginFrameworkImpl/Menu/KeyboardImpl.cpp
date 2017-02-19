#include "CTRPluginFrameworkImpl/Menu/KeyboardImpl.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

namespace CTRPluginFramework
{
    #define USER_VALID  0
    #define USER_ABORT  -1
    #define KEY_ENTER 0xA
    #define KEY_BACKSPACE 0x8

    KeyboardImpl::KeyboardImpl(std::string text)
    {
        _text = text;
        _error = "";
        _userInput = "";

        _isOpen = false;
        _errorMessage = false;
        _askForExit = false;
        _isHex = false;
        _max = 0;
        _layout = HEXADECIMAL;

        _convert = nullptr;
        _compare = nullptr;
    }

    KeyboardImpl::~KeyboardImpl(void)
    {

    }

    void    KeyboardImpl::SetLayout(Layout layout)
    {
        _layout = layout;
        if (layout == HEXADECIMAL)
            _isHex = true;
        else
            _isHex = false;
    }

    void    KeyboardImpl::SetHexadecimal(bool isHex)
    {
        _isHex = isHex;
    }

    void    KeyboardImpl::SetMaxInput(int max)
    {
        _max = max;
    }

    std::string &KeyboardImpl::GetInput(void)
    {
        return(_userInput);
    }

    void    KeyboardImpl::SetConvertCallback(ConvertCallback callback)
    {
        _convert = callback;
    }
    
    void    KeyboardImpl::SetCompareCallback(CompareCallback callback)
    {
        _compare = callback;
    }

    int     KeyboardImpl::Run(void)
    {
        _isOpen = true;
        _userAbort = false;
        _errorMessage = false;
        _askForExit = false;

        bool    mustRelease = false;
        // Check if Process is paused
        if (!ProcessImpl::IsPaused())
        {
            mustRelease  = true;
            ProcessImpl::Pause(false);
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

            if (_userAbort)
            {
                ret = USER_ABORT;
                goto exit;
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
            {
                _isOpen = false;
                ret = 0;
            }
            else if (_askForExit && _errorMessage)
                _askForExit = false;
        }

    exit:
        if (mustRelease)
            ProcessImpl::Play(false);
        return (ret);
    }

    void    KeyboardImpl::_RenderTop(void)
    {
        static Color    black = Color();
        static Color    red = Color(255, 0, 0);
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static IntRect  background(30, 20, 340, 200);

        int   posY = 25;
        int   posX = 40;

        Renderer::SetTarget(TOP);

        // Draw background
        if (Preferences::topBackgroundImage->IsLoaded())
            Preferences::topBackgroundImage->Draw(background.leftTop);
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            Renderer::DrawRect(32, 22, 336, 196, blank, false);            
        }

        Renderer::DrawSysStringReturn(_text.c_str(), posX, posY, 350, blank);

        // IF error
        if (_errorMessage && _error.size() > 0)
        {
            if (posY < 120)
                posY += 48;
            Renderer::DrawSysStringReturn(_error.c_str(), posX, posY, 350, red);
        }
    }

    void    KeyboardImpl::_RenderBottom(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static IntRect  background(20, 20, 280, 200);

        Renderer::SetTarget(BOTTOM);

        /*// Background
        if (Preferences::bottomBackgroundImage->IsLoaded())
            Preferences::bottomBackgroundImage->Draw(background.leftTop);
        else
        {*/
            Renderer::DrawRect(background, black);
           // Renderer::DrawRect(22, 22, 276, 196, blank, false);            
        //}

        // Draw current input

        int   posY = 20;
        int   posX = 25;

        Renderer::DrawSysString(_userInput.c_str(), posX, posY, 300, blank);    

        // Draw keys
        KeyIter iter = _keys.begin();

        for (; iter != _keys.end(); iter++)
        {
            (*iter).Draw();
        }
    }

    void    KeyboardImpl::_ProcessEvent(Event &event)
    {
        if (event.type == Event::KeyPressed)
        {
            if (event.key.code == Key::B)
            {
                _userAbort = true;
            }
        }
    }

    void    KeyboardImpl::_Update(void)
    {
        KeyIter iter = _keys.begin();

        bool isTouchDown = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());

        for (; iter != _keys.end(); iter++)
        {
            (*iter).Update(isTouchDown, touchPos);
        }
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
        KeyIter  end = iter;
        std::advance(end, 6);

        for (; iter != end; iter++)
            (*iter).Enable(false);
    }

    void    KeyboardImpl::_Hexadecimal(void)
    {
        _DigitKeyboard();

        // Disable dot key
        _keys.at(17).Enable(false);

        // Disable Hex keys if users asks so
        if (!_isHex)
        {
            KeyIter  iter = _keys.begin();
            KeyIter  end = iter;
            std::advance(end, 6);

            for (; iter != end; iter++)
                (*iter).Enable(false);
        }
    }

    bool    KeyboardImpl::_CheckKeys(void)
    {
        int ret = -1;

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
                    if (_userInput.length() > 1)
                        _userInput.pop_back();
                    else if (_userInput.length() == 1)
                        _userInput.clear();
                    else
                        return (false);
                    return (true);
                }
                else
                {
                    if (_userInput.length() == 0 && ret == '.')
                        _userInput += "0.";
                    else if (_max == 0 || _userInput.size() < _max)
                        _userInput += ret;
                    return (true); 
                }
            }
        }
        return (false);
    }

    bool    KeyboardImpl::_CheckInput(void)
    {
        if (_layout == QWERTY && _compare != nullptr)
        {
            return (_compare((void *)&_userInput, _error));
        }
        else if (_layout == DECIMAL && _compare != nullptr && _convert != nullptr)
        {
            void *convertedInput = _convert(_userInput, false);
            return (_compare(convertedInput, _error));
        }
        else if (_compare != nullptr && _convert != nullptr)
        {
            void *convertedInput = _convert(_userInput, _isHex ? true : false);
            return (_compare(convertedInput, _error));            
        }
        // In case there's no callback, always consider input as valid
        return (true);
    }
}

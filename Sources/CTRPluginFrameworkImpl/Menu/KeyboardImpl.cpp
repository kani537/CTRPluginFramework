#include "CTRPluginFrameworkImpl/Menu/KeyboardImpl.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "CTRPluginFrameworkImpl/Preferences.hpp"

#include <cmath>

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

        _customKeyboard  = false;
        _displayScrollbar = false;
        _currentPosition = 0;
        _scrollbarSize = 0;
        _scrollCursorSize = 10;
        _scrollSize = 0.f;
        _scrollPosition = 0.f;
        _inertialVelocity = 0.f;
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

    void    KeyboardImpl::Populate(std::vector<std::string> &input)
    {
        _customKeyboard = true;
        _strKeys.clear();

        IntRect box(60, 25, 200, 30);

        for (int i = 0; i < input.size(); i++)
        {
            _strKeys.push_back(TouchKeyString(input[i], box));
            //if (i < 6)
                box.leftTop.y += 35;
        }
    }

    int     KeyboardImpl::Run(void)
    {
        _isOpen = true;
        _userAbort = false;
        _errorMessage = false;
        _askForExit = false;

        // Check if Process is paused
        if (!ProcessImpl::IsPaused())
        {
            _mustRelease  = true;
            ProcessImpl::Pause(false);
        }
        else
            _mustRelease = false;

        int                 ret = -2;
        Event               event;
        EventManager        manager;
        Clock               clock;

        // Construct keyboard
        if (!_customKeyboard)
        {
            if (_layout == QWERTY) { ret = -1; goto exit; }//<-- Currently unsupported
            else if (_layout == DECIMAL) _Decimal();
            else if (_layout == HEXADECIMAL) _Hexadecimal();            
        }

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
            _Update(clock.Restart().AsSeconds());

            Renderer::StartFrame();
            // Render Top Screen
            _RenderTop();
            // Render Bottom Screen
            _RenderBottom();
            Renderer::EndFrame();

            // if it's a standard keyboard
            if (!_customKeyboard)
            {
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
            else
            {
                int  choice = -1;
                bool isSelected = _CheckButtons(choice);

                if (isSelected)
                {
                    ret = choice;
                    _isOpen = false;
                }
            }
        }

    exit:
        if (_mustRelease)
            ProcessImpl::Play(false);
        return (ret);
    }

    void    KeyboardImpl::_RenderTop(void)
    {
        static Color    black = Color();
        static Color    red = Color(255, 0, 0);
        static Color    blank(255, 255, 255);
        static Color    dimGrey(15, 15, 15);
        static IntRect  background1(30, 20, 340, 200);
        static IntRect  background2(50, 30, 300, 180);

        IntRect &background = _mustRelease ? background2 : background1;
        int     maxX = background.leftTop.x + background.size.x;
        int     maxY = background.leftTop.y + background.size.y;

        int   posY =  background.leftTop.y + 5;
        int   posX =  background.leftTop.x + 5;

        Renderer::SetTarget(TOP);

        // Draw background
        if (Preferences::topBackgroundImage->IsLoaded())
            Preferences::topBackgroundImage->Draw(background);
        else
        {
            Renderer::DrawRect2(background, black, dimGrey);
            //Renderer::DrawRect(32, 22, 336, 196, blank, false);            
        }

        Renderer::DrawSysStringReturn(_text.c_str(), posX, posY, maxX, blank, maxY);

        // IF error
        if (_errorMessage && _error.size() > 0)
        {
            if (posY < 120)
                posY += 48;
            Renderer::DrawSysStringReturn(_error.c_str(), posX, posY, maxX, red, maxY);
        }
    }

    void    KeyboardImpl::_RenderBottom(void)
    {
        static Color    black = Color();
        static Color    blank(255, 255, 255);
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

        if (!_customKeyboard)
        {
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
        else
        {
            int max = _strKeys.size() - _currentPosition;
            max = std::min(max, 6);
            for (int i = _currentPosition; i < max && i < _strKeys.size(); i++)
            {
                _strKeys[i].Draw();
            }

            if (!_displayScrollbar)
                return;

            // Draw scroll bar
            static Color dimGrey(105, 105, 105);
            static Color silver(192, 192, 192);

            // Background
            int posX = 282;
            int posY = 25;

            Renderer::DrawLine(posX, posY + 1, 1, silver, _scrollbarSize - 2);
            Renderer::DrawLine(posX + 1, posY, 1, silver, _scrollbarSize);
            Renderer::DrawLine(posX + 2, posY + 1, 1, silver, _scrollbarSize - 2);

            posY += (int)(_scrollPosition);// * _scrollbarSize);

            Renderer::DrawLine(posX, posY + 1, 1, dimGrey, _scrollCursorSize - 2);
            Renderer::DrawLine(posX + 1, posY, 1, dimGrey, _scrollCursorSize);
            Renderer::DrawLine(posX + 2, posY + 1, 1, dimGrey, _scrollCursorSize - 2); 
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

        // Touch / Scroll
        if (event.type == Event::TouchBegan)
        {
            _inertialVelocity = 0;
            _lastTouch = IntVector(event.touch.x, event.touch.y);
            _touchTimer.Restart();
        }

        if (event.type == Event::TouchMoved)
        {
            Time delta = _touchTimer.Restart();

            float moveDistance = (float)(_lastTouch.y - event.touch.y);
            _inertialVelocity = moveDistance / delta.AsSeconds();
            _lastTouch = IntVector(event.touch.x, event.touch.y);
        }

        if (event.type == Event::TouchEnded)
        {
            if (_touchTimer.GetElapsedTime().AsSeconds() > 0.3f)
                _inertialVelocity = 0.f;
        }
    }

    #define INERTIA_SCROLL_FACTOR 0.9f
    #define INERTIA_ACCELERATION 0.98f
    #define INERTIA_THRESHOLD 1.f

    void    KeyboardImpl::_Update(float delta)
    {
        

        bool isTouchDown = Touch::IsDown();
        IntVector touchPos(Touch::GetPosition());

        if (!_customKeyboard)
        {
            KeyIter iter = _keys.begin();

            for (; iter != _keys.end(); iter++)
            {
                (*iter).Update(isTouchDown, touchPos);
            }            
        }
        else
        {
            // Scroll stuff
            _scrollSize = _inertialVelocity * INERTIA_SCROLL_FACTOR * delta;
            _scrollPosition += _scrollSize;
            _inertialVelocity *= INERTIA_ACCELERATION * delta;
            if (std::abs(_inertialVelocity) < INERTIA_THRESHOLD)
                _inertialVelocity = 0.f;

            KeyStringIter iter = _strKeys.begin();
            
            for (; iter != _strKeys.end(); iter++)
            {
                (*iter).Update(isTouchDown, touchPos);
                //(*iter).ScrollDown((int)_scrollSize);
            }
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

    bool    KeyboardImpl::_CheckButtons(int &ret)
    {
        for (int i = 0; i < _strKeys.size(); i++)
        {
            ret = _strKeys[i]();
            if (ret != -1)
            {
                return (true);
            }
        }

        return (false);
    }
}

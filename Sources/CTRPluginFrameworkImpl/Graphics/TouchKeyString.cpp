#include "CTRPluginFrameworkImpl/Graphics/TouchKeyString.hpp"

namespace CTRPluginFramework
{
    TouchKeyString::TouchKeyString(const std::string &content, IntRect ui, bool isEnabled)
    {
        _content = content;
        _uiProperties = ui;
        _posY = _uiProperties.leftTop.y;
        _enabled = isEnabled;

        _isPressed = false;
        _execute = false;

        _contentLength = Renderer::GetTextSize(_content.c_str());

        while ((int)(_contentLength) > ui.size.x)
        {
            if (_content.size() < 1)
                break;
            _content.pop_back();
            _contentLength = Renderer::GetTextSize(_content.c_str());
        }
    }  

    void    TouchKeyString::Enable(bool isEnabled)
    {
        _enabled = isEnabled;
    }

    void    TouchKeyString::Draw(void)
    {
        static Color    background(51, 51, 51); ///< Sort of grey
        const Color     &blank = Color::Blank;
        const Color     &textSe = Color::Black;
        const Color     &pressed = Color::Gainsboro; 
       

        // If key is disabled
        if (!_enabled)
            return;

        if (_isPressed)
        {
            // Background
            Renderer::DrawRect(_uiProperties, pressed);
            int posX = ((_uiProperties.size.x - (int)_contentLength) / 2) + _uiProperties.leftTop.x;
            int posY = ((_uiProperties.size.y - 16) / 2) + _uiProperties.leftTop.y;
            int maxX = _uiProperties.leftTop.x + _uiProperties.size.x;
            Renderer::DrawSysString(_content.c_str(), posX, posY, maxX, textSe);
        }
        else
        {
            Renderer::DrawRect(_uiProperties, background);
            int posX = ((_uiProperties.size.x - (int)_contentLength) / 2) + _uiProperties.leftTop.x;
            int posY = ((_uiProperties.size.y - 16) / 2) + _uiProperties.leftTop.y;
            int maxX = _uiProperties.leftTop.x + _uiProperties.size.x;
            Renderer::DrawSysString(_content.c_str(), posX, posY, maxX, blank);
        }
    }

    void    TouchKeyString::Update(const bool isTouchDown, const IntVector &touchPos)
    {
        if (!_enabled)
            return;

        bool    isTouched = _uiProperties.Contains(touchPos);

        if (_isPressed && !isTouchDown)
        {            
            _isPressed = false;
            _execute = true;
        } 

        if (isTouchDown && isTouched)
        {
            _isPressed = true;
        }
        else
            _isPressed = false;   
    }

    void    TouchKeyString::Scroll(float amount)
    {
        _posY += amount;

        int &posY = _uiProperties.leftTop.y;

        posY = _posY;
    }

    int     TouchKeyString::operator()(void)
    {
        if (_enabled && _execute)
        {
            _execute = false;
            return (1);
        }
        return (-1);
    }
}

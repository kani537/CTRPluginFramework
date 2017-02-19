#include "CTRPluginFrameworkImpl/Graphics/TouchKeyString.hpp"
#include "ctrulib/util/utf.h"

namespace CTRPluginFramework
{
    TouchKeyString::TouchKeyString(std::string content, IntRect ui, bool isEnabled)
    {
        _content = content;
        _uiProperties = ui;
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
        static Color    background(51, 51, 51); // sort of grey
       // static Color    pressed(192, 192, 192); // silver
        static Color    blank(255, 255, 255); // blank
        static Color    textSe(0, 0, 0); // black
        static Color    pressed(225, 225, 225); 
       

        // if key is disabled
        if (!_enabled)
        {
            return;
        }
        else if (_isPressed)
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

    void    TouchKeyString::Update(bool isTouchDown, IntVector touchPos)
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

    void    TouchKeyString::ScrollUp(int amount)
    {
        int &posY = _uiProperties.leftTop.y;

        posY -= amount;
    }

    void    TouchKeyString::ScrollDown(int amount)
    {
        int &posY = _uiProperties.leftTop.y;

        posY += amount;
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

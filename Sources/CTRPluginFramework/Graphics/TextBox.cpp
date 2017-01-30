#include "CTRPluginFramework/Graphics/Textbox.hpp"
#include "CTRPluginFramework/Clock.hpp"
#include "types.h"
#include "ctrulib/util/utf.h"
#include "ctrulib/font.h"

#include <cstdio>

namespace CTRPluginFramework
{
    /*
    ** Constructor
    ***************/
    TextBox::TextBox(std::string &title, std::string &text, IntRect box) :
    _title(title), _text(text), _box(box)
    {
        _currentLine = 0;
        _isOpen = false;
        _maxLines = ((box.size.y - 10) / 16) - 1;        
        _border = IntRect(box.leftTop.x + 2, box.leftTop.y + 2, box.size.x - 4, box.size.y - 4);
        _GetTextInfos();
        if (_newline.size() < _maxLines)
            _maxLines = _newline.size();
    }

    /*
    ** Open
    ************/
    void    TextBox::Open(void)
    {
        _isOpen = true;
    }

    /*
    ** Close
    ***********/
    void    TextBox::Close(void)
    {
        _isOpen = false;
    }

    /*
    ** IsOpen
    ***********/
    bool    TextBox::IsOpen(void)
    {
        return (_isOpen);
    }

    /*
    ** Process Event
    *****************/
    bool    TextBox::ProcessEvent(Event &event)
    {
        if (!_isOpen)
            return (false);
        static Clock inputClock;
        if (event.type == Event::KeyDown)
        {
            switch (event.key.code)
            {
                case Key::DPadUp:
                {
                    if (inputClock.HasTimePassed(Milliseconds(400)))
                    {
                        if (_currentLine > 0)
                            _currentLine--;
                        else
                            _currentLine = _newline.size() - _maxLines;
                        inputClock.Restart();
                    }
                    break;
                }
                case Key::DPadDown:
                {
                    if (inputClock.HasTimePassed(Milliseconds(400)))
                    {
                        if (_currentLine < _newline.size() - _maxLines)
                            _currentLine++;
                        else
                            _currentLine = 0;
                        inputClock.Restart();
                    }
                    break;
                }
                case Key::B:
                {
                    Close();
                    return (false);
                    break;
                }
            }
        }
        return (true);
    }

    /*
    ** Draw
    ***********/
    void    TextBox::Draw(void)
    {
        if (_currentLine >= _newline.size())
            _currentLine = 0;

        int  max = std::min((u32)(_currentLine + _maxLines), (u32)(_newline.size()));

        Color blank = Color(255, 255, 255);
        Color black = Color();
        Color grey = Color(15, 15, 15);

        // Draw Background
        Renderer::DrawRect2(_box, black, grey);
        Renderer::DrawRect(_border, blank, false);

        int posX = _box.leftTop.x + 5;
        int posY = _box.leftTop.y + 5;
        int xLimit = posX + _box.size.x - 5;

        // Draw Title
        int width;
        width = Renderer::DrawSysString((const char *)_title.c_str(), posX, posY, xLimit, blank);
        Renderer::DrawLine(posX, posY, width - posX + 30, blank);
        posY += 7;

        // Draw Text
        for (int i = _currentLine; i < max; i++)
        {
            //char *start = _newline[i];
            //char *end = i == max - 1 ? nullptr :  _newline[i + 1];
            Renderer::DrawSysString(_newline[i], posX, posY, xLimit, blank, 0, _newline[i + 1]);

        }
    }

    /*
    ** Text infos
    ***************/

    char   *TextBox::_GetWordWidth(char *str, float &width, float &extra, int &count)
    {
        width = 0;
        count = 0;
        u32             code;
        ssize_t         units = decode_utf8(&code, (u8 *)str);
        if (units == -1)
            return (nullptr);
        
        while (*str != ' ' && *str != '\n')
        {   
            int                 glyphIndex;
            fontGlyphPos_s      data;
            charWidthInfo_s     *cwi;
            units = decode_utf8(&code, (u8 *)str);

            if (units == -1 || *str == '\0')
                return (nullptr);

            glyphIndex = fontGlyphIndexFromCodePoint(code);         
            Renderer::FontCalcGlyphPos(&data, &cwi, glyphIndex, 0.5f, 0.5f);
            width += ceilf(data.xAdvance) + 1;
            str += units;
            count += units;
        }

        if (*str == ' ')
        {
            u32             code;
            ssize_t         units = decode_utf8(&code, (u8 *)str);
            if (units == -1)
                return (nullptr);
            int             glyphIndex;
            fontGlyphPos_s  data;
            charWidthInfo_s *cwi;

            glyphIndex = fontGlyphIndexFromCodePoint(code);         
            Renderer::FontCalcGlyphPos(&data, &cwi, glyphIndex, 0.5f, 0.5f);
            extra = ceilf(data.xAdvance) + 1;
            str += units;
            count += units;       
        }
        return (str);
    }

    /*
    ** Get text infos
    ******************/
    void     TextBox::_GetTextInfos(void)
    {
        float           w = 0.0f;
        char            *str = (char *)_text.c_str();

        float maxWidth = _border.size.x - 5.f;

        _newline.push_back(str);

        while (*str == '\n')
        {
            _newline.push_back(str);
            str++;
        }

        // Get space width
        /*float           spaceWidth;
       
        ssize_t         units = decode_utf8(&_code, (u8 *)" ");
        int             glyphIndex;
        fontGlyphPos_s  data;
        charWidthInfo_s *cwi;

        glyphIndex = fontGlyphIndexFromCodePoint(_code);         
        Renderer::FontCalcGlyphPos(&data, &cwi, glyphIndex, 0.5f, 0.5f);
        spaceWidth = data.xAdvance + 1;*/

        while (*str != '\0')
        {
            if (*str == '\n')
                str++;
            char *s = str;            
            float line = 0.f;


            while (line < maxWidth && s != nullptr)
            {
                str = s;

                float ww;
                float extra;
                int count;

                s = _GetWordWidth(s, ww, extra, count);
                //s += count;

                line += ww;
                if (s != nullptr && *s == '\n' && line < maxWidth)
                {
                    str = s;
                    break;
                }
                if (line < maxWidth && line + extra >= maxWidth)
                {
                    str = s;
                    break;                                  
                }
                line += extra;  
            }

            if (s != nullptr && *s != '\0')
                _newline.push_back(str);
            else
                break;
            
        }
        //_newline.push_back(str);
        _newline.push_back(nullptr);
    }
}
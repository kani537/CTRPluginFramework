#include "CTRPluginFrameworkImpl/Graphics/NumericTextBox.hpp"
#include "CTRPluginFrameworkImpl/Graphics.hpp"
#include "CTRPluginFramework/Menu/Keyboard.hpp"

#include <cstdio>

namespace CTRPluginFramework
{
    NumericTextBox::NumericTextBox(int posX, int posY, int width, int height) :
    _rectPos(IntRect(posX, posY, width, height)),
    _execute(false),
    _isTouched(false),
    IsEnabled(true),
    IsVisible(true),
    ValueType(Type::Bits32),
    Bits64(0)
    {
        Clear();
    }

    void    NumericTextBox::_UpdateVal(void)
    {
        char buffer[17] = {0};

        switch (ValueType)
        {
            case Type::Bits8: sprintf(buffer, "%08X", Bits8); break;
            case Type::Bits16: sprintf(buffer, "%08X", Bits16); break;
            case Type::Bits32: sprintf(buffer, "%08X", Bits32); break;
            case Type::Bits64: sprintf(buffer, "%016llX", Bits64); break;
            case Type::Float: sprintf(buffer, "%.4f", Single); break;
            case Type::Double: sprintf(buffer, "%.4lf", Double); break;
        }
        _text = buffer;
    }

    void    NumericTextBox::SetValue(u8 val)
    {
        ValueType = Type::Bits8;
        Bits8 = val;
        
        _UpdateVal();
    }

    void    NumericTextBox::SetValue(u16 val)
    {
        ValueType = Type::Bits16;
        Bits16 = val;

        _UpdateVal();
    }

    void    NumericTextBox::SetValue(u32 val)
    {
        ValueType = Type::Bits32;
        Bits32 = val;

        _UpdateVal();
    }

    void    NumericTextBox::SetValue(u64 val)
    {
        ValueType = Type::Bits64;
        Bits64 = val;

        _UpdateVal();
    }

    void    NumericTextBox::SetValue(float val)
    {
        ValueType = Type::Float;
        Single = val;

        _UpdateVal();
    }

    void    NumericTextBox::SetValue(double val)
    {
        ValueType = Type::Double;
        Double = val;

        _UpdateVal();
    }

    void    NumericTextBox::Clear(void)
    {
        Bits64 = 0;

        _UpdateVal();
    }

    void    NumericTextBox::Draw(void)
    {
       // static Color    blank = Color(255, 255, 255);
        Color    &black = Color::Black;
        Color    &gainsboro = Color::Gainsboro;
        Color    &grey = Color::Grey;

        if (!IsVisible)
            return;

        // Draw background
        Renderer::DrawRect(_rectPos, IsEnabled ? gainsboro : grey);

        // Draw text
        int posX = _rectPos.leftTop.x + 5;
        int posY = _rectPos.leftTop.y;

        posY += (_rectPos.size.y - 10) / 2;

        Renderer::DrawString((char *)_text.c_str(), posX, posY, black);
    }

    void    NumericTextBox::Update(bool isTouchDown, IntVector touchPos)
    {
        if (!IsEnabled)
            return;
        if (!_isTouched && isTouchDown && _rectPos.Contains(touchPos))
            _isTouched = true;

        if (_isTouched && isTouchDown && !_rectPos.Contains(touchPos))
            _isTouched = false;
        
        if (_isTouched && !isTouchDown)
        {
            _execute = true;
            _isTouched = false;
        }
    }

    bool    NumericTextBox::operator()(void)
    {
        if (IsEnabled && _execute)
        {
            Keyboard  keyboard;

            keyboard.DisplayTopScreen = false;

           int  out = -1;

           switch (ValueType)
            {
                case Type::Bits8: out = keyboard.Open(Bits8, Bits8); break;
                case Type::Bits16: out = keyboard.Open(Bits16, Bits16); break;
                case Type::Bits32: out = keyboard.Open(Bits32, Bits32); break;
                case Type::Bits64: out = keyboard.Open(Bits64, Bits64); break;
                case Type::Float: out = keyboard.Open(Single, Single); break;
                case Type::Double: out = keyboard.Open(Double, Double); break;
            }

            if (out != -1)
            {
                char buffer[17] = {0};

                switch (ValueType)
                {
                    case Type::Bits8: sprintf(buffer, "%08X", Bits8); break;
                    case Type::Bits16: sprintf(buffer, "%08X", Bits16); break;
                    case Type::Bits32: sprintf(buffer, "%08X", Bits32); break;
                    case Type::Bits64: sprintf(buffer, "%016llX", Bits64); break;
                    case Type::Float: sprintf(buffer, "%.4f", Single); break;
                    case Type::Double: sprintf(buffer, "%.4lf", Double); break;
                }
                _text = buffer;
            }

            _execute = false;
            return (true);
        }
        return (false);
    }

}
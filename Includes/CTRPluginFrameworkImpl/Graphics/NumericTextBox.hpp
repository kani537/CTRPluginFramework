#ifndef CTRPLUGINFRAMEWORKIMPL_NUMERICTEXTBOX_HPP
#define CTRPLUGINFRAMEWORKIMPL_NUMERICTEXTBOX_HPP

#include "CTRPluginFrameworkImpl/Graphics/Vector.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Rect.hpp"
#include <string>
#include <vector>

namespace CTRPluginFramework
{
    class NumericTextBox
    {
    public:

        enum class Type
        {
            Bits8,
            Bits16,
            Bits32,
            Bits64,
            Float,
            Double
        };

        NumericTextBox(int posX, int posY, int width, int height);

        void    SetValue(u8 val);
        void    SetValue(u16 val);
        void    SetValue(u32 val);
        void    SetValue(u64 val);
        void    SetValue(float val);
        void    SetValue(double val);

        void    Clear(void);
        void    Draw(void);
        void    Update(bool isTouchDown, IntVector touchPos);
        bool    operator()(void);

        bool    IsEnabled;
        bool    IsVisible;
        Type    ValueType;

        union
        {
            u8      Bits8;
            u16     Bits16;
            u32     Bits32;
            u64     Bits64;
            float   Single;
            double  Double;
        };

    private:
        bool                        _execute;
        bool                        _isTouched;
        IntRect                     _rectPos;
        std::string                 _text;

        void    _UpdateVal(void);

    };
}

#endif
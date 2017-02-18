#include "CTRPluginFrameworkImpl/Menu/KeyboardImpl.hpp"
#include "CTRPluginFramework/Menu/Keyboard.hpp"

#include <string>

namespace CTRPluginFramework
{
    /*
    ** Convert
    ************/

    void    *ConvertToU8(std::string &input, bool isHex)
    {
        static u8   temp = 0;

        if (input.length() > 0)
            temp = static_cast<u8>(std::stoul(input, nullptr, isHex ? 16 : 10));
        return ((void *)&temp);
    }

    void    *ConvertToU16(std::string &input, bool isHex)
    {
        static u16   temp = 0;

        if (input.length() > 0)
            temp = static_cast<u16>(std::stoul(input, nullptr, isHex ? 16 : 10));
        return ((void *)&temp);
    }

    void    *ConvertToU32(std::string &input, bool isHex)
    {
        static u32   temp = 0;

        if (input.length() > 0)
            temp = static_cast<u32>(std::stoul(input, nullptr, isHex ? 16 : 10));
        return ((void *)&temp);
    }

    void    *ConvertToU64(std::string &input, bool isHex)
    {
        static u64   temp = 0;

        if (input.length() > 0)
            temp = static_cast<u64>(std::stoull(input, nullptr, isHex ? 16 : 10));
        return ((void *)&temp);
    }

    void    *ConvertToFloat(std::string &input, bool isHex)
    {
        static float   temp = 0;

        if (input.length() > 0)
            temp = static_cast<float>(std::stof(input, nullptr));
        return ((void *)&temp);
    }

    void    *ConvertToDouble(std::string &input, bool isHex)
    {
        static double   temp = 0;

        if (input.length() > 0)
            temp = static_cast<double>(std::stod(input, nullptr));
        return ((void *)&temp);
    }

    /*
    ** Compare
    ***********/

    Keyboard::Keyboard(std::string text) : _keyboard(new KeyboardImpl(text))
    {
        _hexadecimal = true;
    }

    Keyboard::~Keyboard(void)
    {

    }

    void    Keyboard::IsHexadecimal(bool isHex)
    {
        _hexadecimal = isHex;
    }

    void    Keyboard::SetCompareCallback(CompareCallback callback)
    {
        _keyboard->SetCompareCallback(callback);
    }

    /*
    ** U8
    ******/

    int     Keyboard::Open(u8 &output)
    {
        _keyboard->SetLayout(Layout::HEXADECIMAL);
        _keyboard->SetHexadecimal(_hexadecimal);
        if (_hexadecimal)
        {
            _keyboard->SetMaxInput(2);
        }
        _keyboard->SetConvertCallback(ConvertToU8);

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            std::string &input = _keyboard->GetInput();
            output = *(static_cast<u8 *>(ConvertToU8(input, _hexadecimal)));
        }
        return (ret);
    }

    int     Keyboard::Open(u8 &output, u8 start)
    {
        _keyboard->SetLayout(Layout::HEXADECIMAL);
        _keyboard->SetHexadecimal(_hexadecimal);
        std::string &input = _keyboard->GetInput();
        if (_hexadecimal)
        {
            _keyboard->SetMaxInput(2);
            char buffer[0x100];
            snprintf(buffer, 0x100, "%X", start);        
            input = buffer;
        }
        else
        {
            char buffer[0x100];
            snprintf(buffer, 0x100, "%d", start);        
            input = buffer;
        }
        _keyboard->SetConvertCallback(ConvertToU8);


        int ret = _keyboard->Run();

        if (ret != -1)
        {
            output = *(static_cast<u8 *>(ConvertToU8(input, _hexadecimal)));
        }
        return (ret);
    }

    /*
    ** U16
    ******/

    int     Keyboard::Open(u16 &output)
    {
        _keyboard->SetLayout(Layout::HEXADECIMAL);
        _keyboard->SetHexadecimal(_hexadecimal);
        if (_hexadecimal)
        {
            _keyboard->SetMaxInput(4);
        }
        _keyboard->SetConvertCallback(ConvertToU16);

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            std::string &input = _keyboard->GetInput();
            output = *(static_cast<u16 *>(ConvertToU16(input, _hexadecimal)));
        }
        return (ret);
    }

    int     Keyboard::Open(u16 &output, u16 start)
    {
        _keyboard->SetLayout(Layout::HEXADECIMAL);
        _keyboard->SetHexadecimal(_hexadecimal);
        std::string &input = _keyboard->GetInput();
        if (_hexadecimal)
        {
            _keyboard->SetMaxInput(4);
            char buffer[0x100];
            snprintf(buffer, 0x100, "%X", start);        
            input = buffer;
        }
        else
        {
            char buffer[0x100];
            snprintf(buffer, 0x100, "%d", start);        
            input = buffer;
        }
        _keyboard->SetConvertCallback(ConvertToU16);

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            output = *(static_cast<u16 *>(ConvertToU16(input, _hexadecimal)));
        }
        return (ret);
    }

    /*
    ** U32
    ******/

    int     Keyboard::Open(u32 &output)
    {
        _keyboard->SetLayout(Layout::HEXADECIMAL);
        _keyboard->SetHexadecimal(_hexadecimal);
        if (_hexadecimal)
        {
            _keyboard->SetMaxInput(8);
        }
        _keyboard->SetConvertCallback(ConvertToU32);

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            std::string &input = _keyboard->GetInput();
            output = *(static_cast<u32 *>(ConvertToU32(input, _hexadecimal)));
        }
        return (ret);
    }

    int     Keyboard::Open(u32 &output, u32 start)
    {
        _keyboard->SetLayout(Layout::HEXADECIMAL);
        _keyboard->SetHexadecimal(_hexadecimal);
        std::string &input = _keyboard->GetInput();
        if (_hexadecimal)
        {
            _keyboard->SetMaxInput(8);
            char buffer[0x100];
            snprintf(buffer, 0x100, "%X", start);        
            input = buffer;
        }
        else
        {
            char buffer[0x100];
            snprintf(buffer, 0x100, "%d", start);        
            input = buffer;
        }
        _keyboard->SetConvertCallback(ConvertToU32);

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            output = *(static_cast<u32 *>(ConvertToU32(input, _hexadecimal)));
        }
        return (ret);
    }

    /*
    ** U64
    ******/

    int     Keyboard::Open(u64 &output)
    {
        _keyboard->SetLayout(Layout::HEXADECIMAL);
        _keyboard->SetHexadecimal(_hexadecimal);
        if (_hexadecimal)
        {
            _keyboard->SetMaxInput(16);
        }
        _keyboard->SetConvertCallback(ConvertToU64);

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            std::string &input = _keyboard->GetInput();
            output = *(static_cast<u64 *>(ConvertToU64(input, _hexadecimal)));
        }
        return (ret);
    }

    int     Keyboard::Open(u64 &output, u64 start)
    {
        _keyboard->SetLayout(Layout::HEXADECIMAL);
        _keyboard->SetHexadecimal(_hexadecimal);
        std::string &input = _keyboard->GetInput();
        if (_hexadecimal)
        {
            _keyboard->SetMaxInput(16);
            char buffer[0x100];
            snprintf(buffer, 0x100, "%llX", start);        
            input = buffer;
        }
        else
        {
            char buffer[0x100];
            snprintf(buffer, 0x100, "%ld", start);        
            input = buffer;
        }
        _keyboard->SetConvertCallback(ConvertToU64);

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            output = *(static_cast<u64 *>(ConvertToU64(input, _hexadecimal)));
        }
        return (ret);
    }

    /*
    ** float
    ******/

    int     Keyboard::Open(float &output)
    {
        _keyboard->SetLayout(DECIMAL);
        _keyboard->SetConvertCallback(ConvertToFloat);

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            std::string &input = _keyboard->GetInput();
            output = *(static_cast<float *>(ConvertToFloat(input, _hexadecimal)));
        }
        return (ret);
    }

    int     Keyboard::Open(float &output, float start)
    {
        _keyboard->SetLayout(DECIMAL);
        _keyboard->SetConvertCallback(ConvertToFloat);

        char buffer[0x100];
        snprintf(buffer, 0x100, "%f", start);
        std::string &input = _keyboard->GetInput();
        input = buffer;

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            output = *(static_cast<float *>(ConvertToFloat(input, _hexadecimal)));
        }
        return (ret);
    }

    /*
    ** double
    ******/

    int     Keyboard::Open(double &output)
    {
        _keyboard->SetLayout(DECIMAL);
        _keyboard->SetConvertCallback(ConvertToDouble);

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            std::string &input = _keyboard->GetInput();
            output = *(static_cast<double *>(ConvertToDouble(input, _hexadecimal)));
        }
        return (ret);
    }

    int     Keyboard::Open(double &output, double start)
    {
        _keyboard->SetLayout(DECIMAL);
        _keyboard->SetConvertCallback(ConvertToDouble);

        char buffer[0x100];
        snprintf(buffer, 0x100, "%lf", start);
        std::string &input = _keyboard->GetInput();
        input = buffer;

        int ret = _keyboard->Run();

        if (ret != -1)
        {
            output = *(static_cast<double *>(ConvertToDouble(input, _hexadecimal)));
        }
        return (ret);
    }

    /*
    ** string
    **********/

    int     Keyboard::Open(std::string &output)
    {
        // TODO
        return (-1);
    }

    int     Keyboard::Open(std::string &output, std::string start)
    {
        // TODO
        return (-1);
    }
}
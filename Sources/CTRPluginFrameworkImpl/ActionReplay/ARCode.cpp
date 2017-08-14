#include "CTRPluginFrameworkImpl/ActionReplay/ARCode.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"

#include <string>


namespace CTRPluginFramework
{
    ARCode::ARCode(const std::string &line, bool &error)
    {
        if (line.size() < 17)
        {
            error = true;
            return; // An error occured
        }

        std::string lStr = line.substr(0, 8);
        std::string rStr = line.substr(9, 8);


        Left = static_cast<u32>(std::stoul(lStr, nullptr, 16));
        Right = static_cast<u32>(std::stoul(rStr, nullptr, 16));
        Type = Left >> 24;

        u8 type = Type >> 4;
        if (type == 0xC || type == 0xD || type == 0xF)
            Left &= 0x00FFFFFF;
        else
        {
            Type &= 0xF0;
            Left &= 0x0FFFFFFF;
        }
        Data = nullptr;
        error = false;
    }

    std::string     ARCode::ToString(void)
    {
        if (Type == 0xE0)
        {
            std::string ret = Utils::Format("%08X %08X\n", Type << 24 | Left, Right);

            u32     size = Right;
            u32     left = 0;
            int     pos = 0;
            int     i = 0;
            bool    space = true;

            while (size >= 4)
            {
                ret += Utils::Format("%08X", (u32 *)&Data[pos]);
                if (space)
                {
                    ret += " ";
                    space = false;
                }
                else
                {
                    ret += "\n";
                    space = true;
                }
                pos += 4;
                size -= 4;
            }
            while (size)
            {
                left |= Data[pos++] << (i++ * 8);
                size--;
            }

            if (space)
                ret += Utils::Format("%08X 00000000", left);
            else
                ret += Utils::Format("%08X", left);
            return (ret);
        }
        return (Utils::Format("%08X %08X", Type << 24 | Left, Right));
    }
}

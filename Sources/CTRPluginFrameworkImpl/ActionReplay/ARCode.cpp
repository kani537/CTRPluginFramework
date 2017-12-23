#include "CTRPluginFrameworkImpl/ActionReplay/ARCode.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"
#include <string>

namespace CTRPluginFramework
{
    namespace ActionReplayPriv
    {
        u32     Str2U32(const std::string &str, bool &error)
        {
            u32 val = 0;
            const u8 *hex = (const u8 *)str.c_str();

            error = false;
            if (str.empty())
                return (val);

            while (*hex)
            {
                u8 byte = (u8)*hex++;

                if (byte >= '0' && byte <= '9') byte = byte - '0';
                else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
                else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
                else ///< Incorrect char
                {
                    error = true;
                    return (0);
                }

                val = (val << 4) | (byte & 0xF);
            }
            return (val);
        }
    }

    ARCode::ARCode(const std::string &line, bool &error)
    {
        if (line.size() < 17)
        {
            error = true;
            return; // An error occured
        }

        std::string lStr = line.substr(0, 8);
        std::string rStr = line.substr(9, 8);


        Left = ActionReplayPriv::Str2U32(lStr, error);
        Right = ActionReplayPriv::Str2U32(rStr, error);

        if (error)
            return;

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

    std::string     ARCode::ToString(void) const
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
                ret += Utils::Format("%08X", *(u32 *)&Data[pos]);
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

            if (space && left)
                ret += Utils::Format("%08X 00000000", left);
            else if (!space)
                ret += Utils::Format("%08X", left);
            return (ret);
        }
        return (Utils::Format("%08X %08X", Type << 24 | Left, Right));
    }

    void    ARCodeContext::Clear(void)
    {
        hasError = false;
        data.clear();
        storage[0] = storage[1] = 0;
        codes.clear();
    }
}

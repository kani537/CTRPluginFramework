#include "types.h"
#include "ctrulib/util/utf.h"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFramework/System/File.hpp"
#include "CTRPluginFramework/Utils.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuFolderImpl.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARCode.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/MenuEntryActionReplay.hpp"
#include <algorithm>
#include <stack>
#include <locale>
#include <cctype>
#include <cstring>

namespace CTRPluginFramework
{
    static bool   IsNotCode(const std::string &line)
    {
        u32 index = 0;
        for (const char c : line)
        {
            if (index != 8 && !std::isxdigit(c))
                return (true);
            ++index;
            if (index >= 16)
                break;
        }
        return (false);
    }

    static bool     LineEndWith(const char c, std::string &line)
    {
        return (line.size() && line[line.size() - 1] == c);
    }

    static inline std::string &Ltrim(std::string &str)
    {
        auto it = std::find_if(str.begin(), str.end(), [](char ch) { return (!std::isspace(ch)); });
        str.erase(str.begin(), it);
        return (str);
    }

    static inline std::string &Rtrim(std::string &str)
    {
        auto it = std::find_if(str.rbegin(), str.rend(), [](char ch) { return (!std::isspace(ch)); });
        str.erase(it.base(), str.end());
        return (str);
    }

    static inline std::string &Trim(std::string &str)
    {
        return (Ltrim(Rtrim(str)));
    }

    static std::string ConvertToUTF8(const std::string &str, bool &error)
    {
        u32     code = 0;
        char    buffer[10] = { 0 };

        if (str[0] != '\\' && str[1] != 'u')
        {
            error = true;
            return (buffer);
        }

        error = false;
        code = ActionReplayPriv::Str2U32(str.substr(2, 4), error);
        if (error)
            return (buffer);
        encode_utf8(reinterpret_cast<u8 *>(buffer), code);
        return (buffer);
    }

    static void     Process(std::string &str)
    {
        // Process our string
        for (u32 i = 0; i < str.size() - 2; i++)
        {
            const char c = str[i];

            if (c == '\\')
            {
                // Check the symbol \n
                if (str[i + 1] == 'n')
                {
                    str[i] = '\n';
                    str = str.erase(i + 1, 1);
                }
                // Check any unicode attempt
                else if (str[i + 1] == 'u')
                {
                    bool error = false;
                    std::string utf8 = ConvertToUTF8(str.substr(i, 6), error);

                    if (utf8.empty() || error) continue;

                    str.erase(i, 6);
                    str.insert(i, utf8);
                }
            }
        }
    }

    static bool    ActionReplay_GetData(std::string &line, ARCodeContext *codectx, int &index)
    {
        if (!codectx)
            return (false);

        bool            error = false;
        u32             *data = reinterpret_cast<u32 *>(&codectx->codes.back().Data[index]);
        std::string     &&leftstr = line.substr(0, 8);
        std::string     &&rightstr = line.substr(9, 8);

        *data++ = ActionReplayPriv::Str2U32(leftstr, error);
        if (error) goto exit;
        *data = ActionReplayPriv::Str2U32(rightstr, error);
        if (error) goto exit;
        index += 8;

    exit:
        codectx->hasError = error;
        return (error);
    }

    using FolderStack = std::stack<MenuFolderImpl*>;

    void    ActionReplay_LoadCodes(MenuFolderImpl *dst)
    {
        File            file("cheats.txt");
        LineReader      reader(file);
        MenuEntryActionReplay   *entry = nullptr;
        FolderStack     folders;
        std::string     line;
        std::string     name;
        std::string     note;
        bool            error = false;
        bool            ecode = false;
        int             count = 0;
        int             index = 0;

        if (!file.IsOpen())
        {
            File::Open(file, Utils::Format("/cheats/%016llX.txt", Process::GetTitleID()));
            if (!file.IsOpen())
                return;
        }

        folders.push(dst);

        // While there's lines in the file
        while (reader(line))
        {
            // Remove spaces in the left
            Ltrim(line);

            // If line is empty
            if (line.empty())
            {
                if (entry)
                    folders.top()->Append(entry);
                entry = nullptr;
                name.clear();
                error = ecode = false;
                continue;
            }

            // If line is not a code
            if (IsNotCode(line) || error)
            {
                // If we found a pattern
                if (line[0] == '[')
                {
                    if (entry)
                        folders.top()->Append(entry);
                    entry = nullptr;

                    Rtrim(line);
                    // If line is a folder opening pattern
                    if (line.substr(0, 3) == "[++")
                    {
                        name = line.substr(3, line.size() - 6);
                        Trim(name);
                        Process(name);
                        folders.push(new MenuFolderImpl(name));
                        continue;
                    }

                    // If line is a folder close pattern
                    if (line == "[--]")
                    {
                        if (folders.size() > 1)
                        {
                            MenuFolderImpl *f = folders.top();
                            folders.pop();
                            folders.top()->Append(f);
                        }

                        continue;
                    }

                    // Else it's a new code pattern
                    name = line.substr(1, line.size() - 2);
                    Trim(name);
                    Process(name);
                    entry = new MenuEntryActionReplay(name);
                    error = false;
                    continue;
                }
                // If we found a note pattern
                if (entry && line[0] == ('{'))
                {
                    Rtrim(line);
                    note = line.substr(1);

                    while (!LineEndWith('}', line))
                    {
                        if (!reader(line))
                            break;
                        note += Rtrim(line);
                    }
                    note.pop_back();
                    Trim(note);
                    Process(note);
                    entry->note = note;
                    continue;
                }

                // Else consider the line as error
                if (entry)
                    entry->context.data += line + "\r\n";
                error = true;
                continue;
            }

            // If there's no entry at this point, skip the line
            if (!entry)
                continue;

            // Add current line to ctx in case of error later
            entry->context.data += line + "\r\n";

            // If we're in E mode
            if (ecode)
            {
                if (!ActionReplay_GetData(line, &entry->context, index))
                {
                    --count;
                    ecode = count > 0;
                }
                else
                    error = true;
                continue;
            }

            // Get ARCode object from line
            ARCode code(line, error);

            // If the code deciphering encountered an error
            if (error)
            {
                entry->context.hasError = error;
                continue;
            }

            // If the code is a E code (data)
            ecode = code.Type == 0xE0;
            if (ecode)
            {
                count = code.Right / 8 + (code.Right % 8 > 0 ? 1 : 0);
                code.Data = new u8[count * 8];
                std::memset(code.Data, 0, count * 8);
                index = 0;
            }

            // Add ARCode to context
            entry->context.codes.push_back(code);
        }

        // Add the last code
        if (entry)
            folders.top()->Append(entry);
    }
}

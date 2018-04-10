#include "CTRPluginFramework/Utils/Utils.hpp"
#include "CTRPluginFramework/System/Directory.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"
#include "CTRPluginFrameworkImpl/Graphics/Icon.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuFolderImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuEntryImpl.hpp"
#include "CTRPluginFrameworkImpl/Menu/Menu.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <random>
#include "ctrulib/util/utf.h"
#include "ctrulib/svc.h"

namespace CTRPluginFramework
{
    std::string Utils::Format(const char* fmt, ...)
    {
        char        buffer[0x100] = { 0 };
        va_list     argList;

        va_start(argList, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, argList);
        va_end(argList);

        return (std::string(buffer));
    }

    std::string Utils::ToHex(u32 x)
    {
        char buf[9] = { 0 };

        sprintf(buf, "%08X", x);

        return (buf);
    }

    std::string     Utils::ToString(float fpval, int precision)
    {
        return (fpval > 999999.f || fpval < -999999.f ? Format(Format("%%.%de", precision).c_str(), fpval) : Format(Format("%%.%df", precision).c_str(), fpval));
    }

    static std::mt19937    g_rng; ///< Engine

    void    InitializeRandomEngine(void)
    {
        // Init the engine with a random seed
        g_rng.seed(svcGetSystemTick());
    }

    u32     Utils::Random(void)
    {
        return (Random(0, 0));
    }

    u32     Utils::Random(u32 min, u32 max)
    {
        if (max == 0)
            max = UINT32_MAX;
        std::uniform_int_distribution<u32>  uniform(min, max);

        return (uniform(g_rng));
    }

    u32     Utils::GetSize(const std::string &str)
    {
        u32     size = str.length();
        u8      buffer[0x100] = { 0 };
        u8      *s = buffer;

        if (!size) return (0);

        std::memcpy(buffer, str.data(), size);

        // Skip UTF8 sig
        if (s[0] == 0xEF && s[1] == 0xBB && s[2] == 0xBF)
            s += 3;

        size = 0;
        while (*s)
        {
            if (*s == 0x18)
            {
                s++;
                continue;
            }

            if (*s == 0x1B)
            {
                s += 4;
                continue;
            }

            u32 code;
            int units = decode_utf8(&code, s);

            if (units == -1)
                break;

            s += units;
            size++;
        }
        return (size);
    }

    static void     ListFolders(MenuFolderImpl &folder, const std::string &filter)
    {
        std::string     &path = folder.note;
        Directory       dir;
        std::vector<std::string>    list;

        int res = Directory::Open(dir, path);

        if (res != 0)
            MessageBox("Error", Utils::Format("%08X - %s", res, path.c_str()))();

        if (path.size() > 1 && path[path.size() - 1] != '/')
            path.append("/");
        if (dir.ListDirectories(list) > 0)
        {
            for (std::string &name : list)
            {
                folder.Append(new MenuFolderImpl(name, path + name));
            }
        }

        list.clear();

        if (dir.ListFiles(list, filter) > 0)
        {
            for (std::string &name : list)
            {
                folder.Append(new MenuEntryImpl(name));
            }
        }
    }

    int Utils::SDExplorer(std::string &out, const std::string &filter)
    {
        Menu            menu("/", "", Icon::DrawFile);
        MenuFolderImpl  &root = *menu.GetRootFolder();

        // Use note to store current path
        root.note = "/";

        // List root's items
        ListFolders(root, filter);

        // Open menu
        int             menuEvent = Nothing;
        Event           event;
        EventManager    eventManager;
        MenuItem        *item;
        Clock           clock;

        do
        {
            menuEvent = Nothing;
            while (eventManager.PollEvent(event) && menuEvent == Nothing)
                menuEvent = menu.ProcessEvent(event, &item);

            if (menuEvent == FolderChanged)
            {
                MenuFolderImpl *folder = reinterpret_cast<MenuFolderImpl *>(item);

                if (!folder->ItemsCount())
                {
                    ListFolders(*folder, filter);
                }
            }

            menu.Update(clock.Restart());
            menu.Draw();
            Renderer::EndFrame();
        } while (menuEvent != EntrySelected && menuEvent != MenuClose);

        if (menuEvent == MenuClose)
            return -1;

        out = menu.GetFolder()->note;
        out.append("/");
        out.append(menu.GetSelectedItem()->name);
        return 0;
    }

    u32     Utils::RemoveLastChar(std::string &str)
    {
        u32     size = str.length();
        u8      buffer[0x100] = { 0 };
        u8      *s = buffer;

        if (!size) return (0);

        std::memcpy(buffer, str.data(), size);

        // Skip UTF8 sig
        if (s[0] == 0xEF && s[1] == 0xBB && s[2] == 0xBF)
            s += 3;

        u32 code = 0;
        int units = 0;

        while (*s)
        {
            // End color pattern
            if (*s == 0x18)
            {
                // if it's the last char, remove it along with previous char
                if (!*(s + 1))
                {
                    if (units)
                    {
                        // go back to previous char
                        s -= units;
                    }

                    *s = 0;
                    str = reinterpret_cast<char *>(buffer);
                    return (code);
                }
                // Else just skip it
                s++;
                units++;
                continue;
            }

            // Start color pattern
            if(*s == 0x1B)
            {
                // If it's the last char
                if (*(s + 1) && *(s + 2) && *(s + 3) && !*(s + 4))
                {
                    // remove it along with previous char
                    if (units)
                    {
                        // go back to previous char
                        s -= units;
                    }

                    *s = 0;
                    str = reinterpret_cast<char *>(buffer);
                    return (code);
                }
                // Else skip it
                s += 4;
                units += 4;
                continue;
            }

            units = decode_utf8(&code, s);

            if (units == -1)
                break;
            if (*(s + units))
                s += units;
            else
            {
                *s = 0;
                str = reinterpret_cast<char *>(buffer);
                return (code);
            }
        }
        return (0);
    }
}

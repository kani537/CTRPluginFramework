#include "CTRPluginFramework.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARCode.hpp"
#include "CTRPluginFrameworkImpl/ActionReplay/ARHandler.hpp"
#include <algorithm>
#include <locale>

namespace CTRPluginFramework
{
   MenuFolder  *g_folder = new MenuFolder("Action Replay");

    struct AREntry
    {
        ARCodeVector     arcodes;
        u32     Storage[2];
    };

    void    ExecuteARCodes(MenuEntry *entry)
    {
        AREntry *ar = static_cast<AREntry *>(entry->GetArg());

        ARHandler::Execute(ar->arcodes, ar->Storage);
    }

    MenuEntry *CreateAREntry(const std::string &name, ARCodeVector &arcodes)
    {
        MenuEntry *entry = new MenuEntry(name, ExecuteARCodes);

        AREntry *ar = new AREntry;

        ar->Storage[0] = 0;
        ar->Storage[1] = 0;
        ar->arcodes = arcodes;

        entry->SetArg(ar);
        return (entry);
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    void    LineReadTest(MenuEntry *entry)
    {

        File        file;
        LineReader  reader(file);
        std::string filename;

        Process::GetTitleID(filename);
        filename += ".txt";
        if (File::Open(file, filename) == 0)
        {
            std::string line;

            std::string     name;
            ARCodeVector    arcodes;

            bool error;
            bool ecode = false;
            int  count = 0;
            int index = 0;

            while (reader(line))
            {
                if (line.empty())
                {
                    if (!name.empty() && !arcodes.empty())
                        *g_folder += CreateAREntry(name, arcodes);
                    arcodes.clear();
                    name.clear();
                    ecode = false;
                    continue;
                }
                // If we're in emode
                if (ecode)
                {
                    u32  *data = reinterpret_cast<u32 *>(&arcodes.back().Data[index]);
                    std::string lStr = line.substr(0, 8);
                    std::string rStr = line.substr(9, 8);
                    u32 left = static_cast<u32>(std::stoul(lStr, nullptr, 16));
                    u32 right = static_cast<u32>(std::stoul(rStr, nullptr, 16));

                    *data++ = left;
                    *data = right;
                    index += 8;
                    count--;
                    if (count == 0)
                    {
                        ecode = false;
                    }

                    continue;
                }

                // If we found a name pattern
                if (line[0] == '[')
                {
                    if (!name.empty() && !arcodes.empty())
                        *g_folder += CreateAREntry(name, arcodes);
                    arcodes.clear();
                    name.clear();

                    rtrim(line);
                    name = line.substr(1, line.size() - 2);
                    continue;
                }

                ARCode code(line, error);

                if (error)
                {
                    OSD::Notify("Error: " + line);
                    break;
                }

                if (code.Type == 0xE0)
                {
                    ecode = true;
                    count = code.Right / 8 + (code.Right % 8 != 0 ? 1 : 0);
                    code.Data = new u8[count * 8];
                    index = 0;
                }

                arcodes.push_back(code);
            }

            if (!name.empty() && !arcodes.empty())
                *g_folder += CreateAREntry(name, arcodes);

            MessageBox(Utils::Format("Found %d cheats !", g_folder->ItemsCount()))();
        }
    }
}

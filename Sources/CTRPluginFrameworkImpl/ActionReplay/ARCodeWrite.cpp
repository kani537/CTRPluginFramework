#include "CTRPluginFrameworkImpl/ActionReplay/MenuEntryActionReplay.hpp"
#include "CTRPluginFrameworkImpl/Menu/MenuFolderImpl.hpp"
#include "CTRPluginFramework/Utils/LineWriter.hpp"
#include "CTRPluginFramework/Utils/Utils.hpp"

namespace CTRPluginFramework
{
    #define endl LineWriter::endl()
    class MenuEntryActionReplay;

    static bool     ActionReplay_WriteName(LineWriter &writer, const MenuItem *item)
    {
        if (!item || item->name.empty())
            return false;
        writer << "[" << item->name << "]" << endl;
        return true;
    }

    static bool     ActionReplay_WriteNote(LineWriter &writer, const MenuItem *item)
    {
        if (!item || item->note.empty())
            return false;
        writer << "{" << item->note << "}" << endl;
        return true;
    }

    static bool     ActionReplay_WriteCode(LineWriter &writer, MenuEntryActionReplay *entry)
    {
        if (!entry || entry->context.codes.empty())
            return false;

        for (ARCode &code : entry->context.codes)
        {
            writer << code.ToString() << endl;

            if (!code.Data.empty())
            {
                for (u32 i = 0; i < code.Data.size(); i += 2)
                {
                    writer << Utils::Format("%08X %08X", code.Data[i], code.Data[i + 1]) << endl;
                }
            }
        }
        return true;
    }

    static bool     ActionReplay_WriteFolderToFile(LineWriter &writer, MenuFolderImpl &folder)
    {
        writer << "[++" << folder.name << "++]" << endl << endl;

        for (u32 i = 0; i < folder.ItemsCount(); ++i)
            ActionReplay_WriteToFile(writer, folder[i]);

        writer << "[--]" << endl << endl;
        return true;
    }

    bool    ActionReplay_WriteToFile(LineWriter &writer, MenuItem *item)
    {
        if (!item)
            return (false);

        if (item->IsFolder())
            return ActionReplay_WriteFolderToFile(writer, item->AsMenuFolderImpl());
        else ///< Code
        {
            MenuEntryActionReplay *ar = reinterpret_cast<MenuEntryActionReplay *>(item);

            if (!ActionReplay_WriteName(writer, item)) goto error;
            if (!ActionReplay_WriteCode(writer, ar)) goto error;
            ActionReplay_WriteNote(writer, item);
            writer << endl;
        }

        return true;
    error:
        return false;
    }
}

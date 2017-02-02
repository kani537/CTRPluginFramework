#include "CTRPluginFramework/Folder.hpp"
#include "CTRPluginFramework/File.hpp"

#include "ctrulib/util/utf.h"
#include "ctrulib/result.h"
#include <limits.h>
#include <cstring>

namespace CTRPluginFramework
{
    extern FS_Archive   _sdmcArchive;

    int     File::Rename(std::string oldPath, std::string newPath)
    {
        FS_Path oldFsPath = Directory::_SdmcUtf16Path(oldPath);
        FS_Path newFsPath;
        uint16_t    path[PATH_MAX + 1] = {0};

        if (oldFsPath.data == nullptr)
            return (-1);

        std::memcpy(path, oldFsPath.data, PATH_MAX);
        oldFsPath.data = (u8 *)path;

        newFsPath = Directory::_SdmcUtf16Path(newPath);
        if (newFsPath.data == nullptr)
            return (-1);

        Result res;

        res = FSUSER_RenameFile(_sdmcArchive, oldFsPath, _sdmcArchive, newFsPath);
        if (R_SUCCEEDED(res))
            return (0);

        return (res);
    }

    int     File::Remove(std::string path)
    {
        FS_Path fsPath = Directory::_SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (-1);

        Result res;

        res = FSUSER_DeleteFile(_sdmcArchive, fsPath);
        if (R_SUCCEEDED(res))
            return (0);

        return (res);
    }
}
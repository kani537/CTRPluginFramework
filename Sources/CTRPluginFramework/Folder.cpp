#include "CTRPluginFramework/Folder.hpp"
#include "ctrulib/util/utf.h"
#include "ctrulib/result.h"
#include <limits.h>
#include <cstring>

namespace CTRPluginFramework
{
    std::string         Folder::_workingDirectory;
    extern FS_Archive   sdmcArchive;

    int  Folder::_SdmcFixPath(std::string &path)
    {
        ssize_t         units;
        uint32_t        code;
        std::string     fixPath;

        const uint8_t *p = (const uint8_t *)path.c_str();

        // Move the path pointer to the start of the actual path
        int     offset = 0;
        do
        {
            units = decode_utf8(&code, p);
            if(units < 0)
            {
                //r->_errno = EILSEQ;
                return (-1);
            }
            p += units;
            offset += units;
        } while(code != ':' && code != 0);

        // We found a colon; p points to the actual path
        if(code == ':')
            path = path.substr(offset);

        // Make sure there are no more colons and that the
        // remainder of the filename is valid UTF-8
        p = (const uint8_t*)path.c_str();
        do
        {
            units = decode_utf8(&code, p);
            if(units < 0)
            {
                //r->_errno = EILSEQ;
                return (-1);
            }

            if(code == ':')
            {
                //r->_errno = EINVAL;
                return (-1);
            }

            p += units;
        } while(code != 0);

        if(path[0] == '/')
            return (0);
        else
        {
            fixPath = _workingDirectory;
            fixPath += path;
            path = fixPath;
        }

        if(path.size() >= PATH_MAX)
        {
            //__fixedpath[PATH_MAX] = 0;
            //r->_errno = ENAMETOOLONG;
            return (-1);
        }
        return (0);
    }

    FS_Path     Folder::_SdmcUtf16Path(std::string &path)
    {
        ssize_t     units;
        FS_Path     fspath;
        static      uint16_t    utf16Path[PATH_MAX + 1] = {0};

        fspath.data = nullptr;

        if(_SdmcFixPath(path) == -1)
            return (fspath);

        units = utf8_to_utf16(utf16Path, (const uint8_t*)path.c_str(), PATH_MAX);
        if(units < 0)
        {
            //r->_errno = EILSEQ;
            return (fspath);
        }
        if(units >= PATH_MAX)
        {
            //r->_errno = ENAMETOOLONG;
            return (fspath);
        }

        utf16Path[units] = 0;

        fspath.type = PATH_UTF16;
        fspath.size = (units + 1) * sizeof(uint16_t);
        fspath.data = (const u8 *)utf16Path;

        return (fspath);
    }

    /*
    ** Static method
    ******************/

    /*
    ** CWD
    ******************/

    int     Folder::ChangeWorkingDirectory(std::string &path)
    {
        FS_Path fsPath = _SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (-1);

        Handle file;
        Result res;

        res = FSUSER_OpenDirectory(&file, sdmcArchive, fsPath);
        if (R_SUCCEEDED(res))
        {
            FSDIR_Close(file);
            _workingDirectory = path;
            return (0);
        }

        return (res);
        
    }

    /*
    ** Create
    ***********/
    int     Folder::Create(std::string &path)
    {
        FS_Path fsPath = _SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (-1);

        Result res;

        res = FSUSER_CreateDirectory(sdmcArchive, fsPath, 0);
        if(res == 0xC82044BE)
            return (1);
        else if (R_SUCCEEDED(res))
            return (0);
        return (res);
    }

    /*
    ** Remove
    ************/
    int     Folder::Remove(std::string &path)
    {
        FS_Path fsPath = _SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (-1);

        Result res;

        res = FSUSER_DeleteDirectory(sdmcArchive, fsPath);
        if (R_SUCCEEDED(res))
            return (0);

        return (res);
    }

    /*
    ** Rename
    ***********/
    int     Folder::Rename(std::string &oldPath, std::string &newPath)
    {
        uint16_t    oldpath[PATH_MAX + 1] = {0};

        FS_Path     fsOldPath; 
        FS_Path     fsNewPath;

        fsOldPath = _SdmcUtf16Path(oldPath);
        if (fsOldPath.data == nullptr)
            return (-1);

        std::memcpy(oldpath, fsOldPath.data, sizeof(oldpath));
        fsOldPath.data = (const u8 *)oldpath;

        fsNewPath = _SdmcUtf16Path(newPath);
        if (fsNewPath.data == nullptr)
            return (-1);

        Result res;

        res = FSUSER_RenameDirectory(sdmcArchive, fsOldPath, sdmcArchive, fsNewPath);
        if (R_SUCCEEDED(res))
            return (0);

        return (res);
    }   

    /*
    ** IsExists
    ************/
    int    Folder::IsExists(std::string &path)
    {
        FS_Path     fsPath;

        fsPath = _SdmcUtf16Path(path);
        if (fsPath.data == nullptr)
            return (-1);

        Result res;
        Handle handle;

        res = FSUSER_OpenDirectory(&handle, sdmcArchive, fsPath);
        if (R_SUCCEEDED(res))
        {
            FSDIR_Close(handle);
            return (1);
        }

        return (0);
    }

    /*
    ** Open
    **********/
    int     Folder::Open(Folder &output, std::string &path, bool create)
    {
        FS_Path fsPath;
        std::string bakpath = path;
        fsPath = _SdmcUtf16Path(path);
        if (fsPath.data == nullptr)
            return (-1);

        Result res;
        Handle handle;

        res = FSUSER_OpenDirectory(&handle, sdmcArchive, fsPath);
        if (R_FAILED(res) && create)
        {
            if (create)
            {
                if (Create(path) != 0)
                {
                    return (res);
                }
                res = FSUSER_OpenDirectory(&handle, sdmcArchive, fsPath);
                if (R_FAILED(res))
                    return (res);                
            }
            else
                return (res);
        }

        output = Folder(bakpath, handle);
        return (0);
    }

    Folder::Folder  (std::string &path, Handle &handle) :
    _path(path), _handle(handle)
    {
    }

    /*
    ** Close
    ***********/
    int     Folder::Close(void)
    {
        Result res;

        res = FSDIR_Close(_handle);
        if (R_SUCCEEDED(res))
            return (0);
        return (res);
    }

    /*
    ** Open a file
    ****************/
    int     Folder::OpenFile(File &output, std::string &path, bool create)
    {
        std::string fullPath;
        FS_Path fsPath;

        if (path[0] != '/')
            fullPath = _path + "/" + path;
        else
            fullPath = _path + path;

        fsPath = _SdmcUtf16Path(fullPath);
        if (fsPath.data == nullptr)
            return (-1);

        int mode = File::READ | File::WRITE;
        if (create)
            mode |= File::CREATE; 
        
        int res;
        res = File::Open(output, fullPath, mode);
        return (res);
    }
}
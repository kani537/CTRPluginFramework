#include "CTRPluginFramework/System/Directory.hpp"
#include "ctrulib/util/utf.h"
#include "ctrulib/result.h"
#include <limits.h>
#include <cstring>
#include "CTRPluginFrameworkImpl/System/Heap.hpp"
#include <algorithm>
#include "ctrulib/svc.h"

namespace CTRPluginFramework
{
    extern FS_Archive   _sdmcArchive;
    static std::string  _workingDirectory = "";

    namespace   _Path
    {
        int     SdmcFixPath(std::string &path)
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
                if (units < 0)
                {
                    //r->_errno = EILSEQ;
                    return (-1);
                }
                p += units;
                offset += units;
            } while (code != ':' && code != 0);

            // We found a colon; p points to the actual path
            if (code == ':')
                path = path.substr(offset);

            // Make sure there are no more colons and that the
            // remainder of the filename is valid UTF-8
            p = (const uint8_t*)path.c_str();
            do
            {
                units = decode_utf8(&code, p);
                if (units < 0)
                {
                    //r->_errno = EILSEQ;
                    return (-1);
                }

                if (code == ':')
                {
                    //r->_errno = EINVAL;
                    return (-1);
                }

                p += units;
            } while (code != 0);

            if (path[0] == '/')
                return (0);
            else
            {
                fixPath = _workingDirectory;
                fixPath += path;
                path = fixPath;
            }

            if (path.size() >= PATH_MAX)
            {
                //__fixedpath[PATH_MAX] = 0;
                //r->_errno = ENAMETOOLONG;
                return (-1);
            }
            return (0);
        }

        FS_Path     SdmcUtf16Path(std::string path)
        {
            ssize_t     units;
            FS_Path     fspath = { PATH_EMPTY, 0, nullptr };
            static      uint16_t    utf16Path[PATH_MAX + 1] = { 0 };

            if (_Path::SdmcFixPath(path) == -1)
                return (fspath);

            units = utf8_to_utf16(utf16Path, (const uint8_t*)path.c_str(), PATH_MAX);
            if (units < 0)
            {
                //r->_errno = EILSEQ;
                return (fspath);
            }
            if (units >= PATH_MAX)
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
    }

    /*
    ** Static methods
    ******************/

    /*
    ** CWD
    ******************/

    int     Directory::ChangeWorkingDirectory(const std::string &path)
    {
        FS_Path fsPath = _Path::SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (INVALID_PATH);

        Handle file;
        Result res;

        res = FSUSER_OpenDirectory(&file, _sdmcArchive, fsPath);
        if (R_SUCCEEDED(res))
        {
            FSDIR_Close(file);
            _workingDirectory = path;
            return (SUCCESS);
        }

        return (INVALID_PATH);

    }

    /*
    ** Create
    ***********/
    int     Directory::Create(const std::string &path)
    {
        FS_Path fsPath = _Path::SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (INVALID_PATH);

        Result res;

        res = FSUSER_CreateDirectory(_sdmcArchive, fsPath, 0);
        if(res == 0xC82044BE)
            return (1);
        if (R_SUCCEEDED(res))
            return (SUCCESS);
        return (res);
    }

    /*
    ** Remove
    ************/
    int     Directory::Remove(const std::string &path)
    {
        FS_Path fsPath = _Path::SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (INVALID_PATH);

        Result res;

        res = FSUSER_DeleteDirectory(_sdmcArchive, fsPath);
        if (R_SUCCEEDED(res))
            return (SUCCESS);

        return (res);
    }

    /*
    ** Rename
    ***********/
    int     Directory::Rename(const std::string &oldPath, const std::string &newPath)
    {
        uint16_t    oldpath[PATH_MAX + 1] = {0};

        FS_Path     fsOldPath;
        FS_Path     fsNewPath;

        fsOldPath = _Path::SdmcUtf16Path(oldPath);
        if (fsOldPath.data == nullptr)
            return (INVALID_PATH);

        std::memcpy(oldpath, fsOldPath.data, sizeof(oldpath));
        fsOldPath.data = (const u8 *)oldpath;

        fsNewPath = _Path::SdmcUtf16Path(newPath);
        if (fsNewPath.data == nullptr)
            return (INVALID_PATH);

        Result res;

        res = FSUSER_RenameDirectory(_sdmcArchive, fsOldPath, _sdmcArchive, fsNewPath);
        if (R_SUCCEEDED(res))
            return (SUCCESS);

        return (res);
    }

    /*
    ** IsExists
    ************/
    int    Directory::IsExists(const std::string &path)
    {
        FS_Path     fsPath;

        fsPath = _Path::SdmcUtf16Path(path);
        if (fsPath.data == nullptr)
            return (INVALID_PATH);

        Result res;
        Handle handle;

        res = FSUSER_OpenDirectory(&handle, _sdmcArchive, fsPath);
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
    int     Directory::Open(Directory &output, const std::string &path, bool create)
    {
        if (output._isOpen) output.Close();
        output._path.clear();
        output._handle = 0;
        output._isOpen = false;
        output._list.clear();

        FS_Path fsPath = _Path::SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (INVALID_PATH);

        Result res;
        Handle handle;

        res = FSUSER_OpenDirectory(&handle, _sdmcArchive, fsPath);
        if (R_FAILED(res))
        {
            if (create)
            {
                if ((res = Create(path)) != 0)
                    return (res);
                res = FSUSER_OpenDirectory(&handle, _sdmcArchive, fsPath);
                if (R_FAILED(res))
                    return (res);
            }
            else
                return (res);
        }

        output._path = path;
        _Path::SdmcFixPath(output._path);
        output._handle = handle;
        output._isOpen = true;

        return (res);
    }

    /*
    ** Close
    ***********/
    int     Directory::Close(void) const
    {
        if (!_isOpen) return (NOT_OPEN);

        Result res;

        res = FSDIR_Close(_handle);
        if (R_SUCCEEDED(res))
        {
            _isOpen = false;
            return (SUCCESS);
        }

        return (res);
    }

    /*
    ** Open a file
    ****************/
    int     Directory::OpenFile(File &output, const std::string &path, int mode) const
    {
        if (!_isOpen) return (NOT_OPEN);

        std::string fullPath;
        FS_Path fsPath;

        if (path[0] != '/')
            fullPath = _path + "/" + path;
        else
            fullPath = _path + path;

        fsPath = _Path::SdmcUtf16Path(fullPath);
        if (fsPath.data == nullptr)
            return (INVALID_PATH);

        return (File::Open(output, fullPath, mode));
    }

    int     Directory::_List(void) const
    {
        FS_DirectoryEntry   *entry = (FS_DirectoryEntry *)Heap::Alloc(sizeof(FS_DirectoryEntry) * 100);
        u32                 entriesNb = 0;
        u8                  filename[PATH_MAX + 1];
        int                 units;

        if (entry != nullptr)
        while (R_SUCCEEDED(FSDIR_Read(_handle, &entriesNb, 100, entry)))
        {
            if (entriesNb == 0)
                break;

            for (u32 i = 0; i < entriesNb; ++i)
            {
                // Convert name from utf16 to utf8
                units = utf16_to_utf8(filename, entry[i].name, PATH_MAX);
                if (units == -1)
                    continue;
                filename[units] = 0;
                _list.push_back(DirectoryEntry(entry[i].attributes, filename));
            }
        }

        if (entry == nullptr || _list.empty())
        {
            Heap::Free(entry);
            return (-1);
        }

        // Sort the file list
        std::sort(_list.begin(), _list.end(), [](const DirectoryEntry &lhs, const DirectoryEntry &rhs )
        {
            u8      *left = (u8 *)lhs.name.c_str();
            u8      *right = (u8 *)rhs.name.c_str();
            u32     leftCode;
            u32     rightCode;

            do
            {
                ssize_t  leftUnits = decode_utf8(&leftCode, left);
                ssize_t  rightUnits = decode_utf8(&rightCode, right);

                if (leftUnits == -1 || rightUnits == -1)
                    break;
                left += leftUnits;
                right += rightUnits;
            } while (leftCode == rightCode && leftCode != 0 && rightCode != 0);

            return (leftCode < rightCode);
        });
        Heap::Free(entry);
        return (0);
    }

    Directory::DirectoryEntry::DirectoryEntry(u32 attrib, u8 *fname) :
    attributes{ attrib }, name{reinterpret_cast<char *>(fname)}
    {
    }

    /*
    ** List files
    ***************/
    int     Directory::ListFiles(std::vector<std::string> &files, const std::string &pattern) const
    {
        if (!_isOpen) return (NOT_OPEN);

        const bool patternCheck = !pattern.empty();

        if (_list.empty())
            _List();

        int   count = files.size();

        for (DirectoryEntry &entry : _list)
        {
            // If entry is a folder, continue
            if (entry.attributes & 1)
                continue;

            std::string &fn = entry.name;
            if (patternCheck && fn.find(pattern) == std::string::npos)
                continue;

            files.push_back(fn);
        }
        return (files.size() - count);
    }

    /*
    ** List folders
    *****************/
    int     Directory::ListDirectories(std::vector<std::string> &folders, const std::string &pattern) const
    {
        if (!_isOpen) return (NOT_OPEN);

        const bool patternCheck = !pattern.empty();

        if (_list.empty())
            _List();

        int count = folders.size();

        for (DirectoryEntry &entry : _list)
        {
            // If entry is a file, continue
            if (!(entry.attributes & 1))
                continue;

            std::string &fn = entry.name;
            if (patternCheck && fn.find(pattern) == std::string::npos)
                continue;
            folders.push_back(fn);
        }
        return (folders.size() - count);
    }

    std::string     Directory::GetName(void) const
    {
        int pos = _path.rfind("/");

        if (pos != std::string::npos)
            return (_path.substr(pos + 1));

        return (_path); ///< Better return path than nothing
    }

    std::string     Directory::GetFullName(void) const
    {
        return (_path);
    }

    bool    Directory::IsOpen(void) const
    {
        return (_isOpen);
    }

    Directory::Directory(void) :
    _handle(0), _isOpen(false)
    {
    }

    Directory::Directory(const std::string &path, bool create) :
        _handle(0), _isOpen(false)
    {
        Open(*this, path, create);
    }

    Directory::~Directory()
    {
        _list.clear();
        Close();
    }
}

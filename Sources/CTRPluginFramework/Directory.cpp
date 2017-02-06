#include "CTRPluginFramework/Directory.hpp"
#include "ctrulib/util/utf.h"
#include "ctrulib/result.h"
#include <limits.h>
#include <cstring>
#include <algorithm>    

namespace CTRPluginFramework
{
    std::string         Directory::_workingDirectory = "";
    extern FS_Archive   _sdmcArchive;

    int  Directory::_SdmcFixPath(std::string &path)
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

    FS_Path     Directory::_SdmcUtf16Path(std::string path)
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

    int     Directory::ChangeWorkingDirectory(std::string path)
    {
        FS_Path fsPath = _SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (-1);

        Handle file;
        Result res;

        res = FSUSER_OpenDirectory(&file, _sdmcArchive, fsPath);
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
    int     Directory::Create(std::string path)
    {
        FS_Path fsPath = _SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (-1);

        Result res;

        res = FSUSER_CreateDirectory(_sdmcArchive, fsPath, 0);
        if(res == 0xC82044BE)
            return (1);
        else if (R_SUCCEEDED(res))
            return (0);
        return (res);
    }

    /*
    ** Remove
    ************/
    int     Directory::Remove(std::string path)
    {
        FS_Path fsPath = _SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (-1);

        Result res;

        res = FSUSER_DeleteDirectory(_sdmcArchive, fsPath);
        if (R_SUCCEEDED(res))
            return (0);

        return (res);
    }

    /*
    ** Rename
    ***********/
    int     Directory::Rename(std::string oldPath, std::string newPath)
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

        res = FSUSER_RenameDirectory(_sdmcArchive, fsOldPath, _sdmcArchive, fsNewPath);
        if (R_SUCCEEDED(res))
            return (0);

        return (res);
    }   

    /*
    ** IsExists
    ************/
    int    Directory::IsExists(std::string path)
    {
        FS_Path     fsPath;

        fsPath = _SdmcUtf16Path(path);
        if (fsPath.data == nullptr)
            return (-1);

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
    int     Directory::Open(Directory &output, std::string path, bool create)
    {
        FS_Path fsPath;
        std::string bakpath = path;
        fsPath = _SdmcUtf16Path(path);
        if (fsPath.data == nullptr)
            return (-1);

        Result res;
        Handle handle = 0;

        res = FSUSER_OpenDirectory(&handle, _sdmcArchive, fsPath);
        if (R_FAILED(res) && create)
        {
            if (create)
            {
                if (Create(path) != 0)
                {
                    return (res);
                }
                res = FSUSER_OpenDirectory(&handle, _sdmcArchive, fsPath);
                if (R_FAILED(res))
                    return (res);             
            }
            else
                return (res);
        }
        output._path = bakpath;
        output._handle = handle;
        output._isListed = false;

        //output = Folders(bakpath, handle);// = Folders(bakpath, handle);
        return (res);
    }

    Directory::Directory(std::string &path, Handle &handle) :
    _path(path)
    {
        _handle = handle;
    }

    /*
    ** Close
    ***********/
    int     Directory::Close(void)
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
    int     Directory::OpenFile(File &output, std::string path, bool create)
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
        //res = File::Open(output, fullPath, mode);
        return (res);
    }

    /*
    ** List files
    ***************/
    #define MAX_ENTRIES 20

    bool SortByName(const FS_DirectoryEntry &lhs, const FS_DirectoryEntry &rhs) 
    { 
        u8      *left = (u8 *)lhs.name;
        u8      *right = (u8 *)rhs.name;
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
    }

    int     Directory::_List(void)
    {

        FS_DirectoryEntry   entry;
        Result              res;
        u32                 units;
        u32                 entriesNb = 0;
        u8                  filename[PATH_MAX + 1] = {0};

        while (R_SUCCEEDED(FSDIR_Read(_handle, &entriesNb, 1, &entry)))
        {
            if (entriesNb == 0)
                return (0);

            _list.push_back(entry);           
        }

        std::sort(_list.begin(), _list.end(), SortByName);

        _isListed = true;
        return (res);
    }
    int     Directory::ListFiles(std::vector<std::string> &files, std::string pattern)
    {
        bool patternCheck = (pattern.size() > 0);
       // FS_DirectoryEntry   entries[MAX_ENTRIES] = {0};
        FS_DirectoryEntry   entry;
        Result              res;
        u32                 units;
        u32                 entriesNb = 0;
        u8                  filename[PATH_MAX + 1] = {0};

        if (!_isListed)
            _List();

        for (int i = 0; i < _list.size(); i++)
        {
            entry = _list[i];
            //entry = &entries[i];
            if (entry.attributes & 1)
                continue;
            std::memset(filename, 0, sizeof(filename));
            units = utf16_to_utf8(filename, entry.name, PATH_MAX);
            if (units < 0)
                continue;
            std::string fn = (char *)filename;
            if (patternCheck && fn.find(pattern) == std::string::npos)
                continue;
            files.push_back(fn);           
        }
        return (res);
    }

    /*
    ** List folders
    *****************/
    int     Directory::ListFolders(std::vector<std::string> &folders, std::string pattern)
    {
        bool patternCheck = (pattern.size() > 0);
        //FS_DirectoryEntry   entries[MAX_ENTRIES] = {0};
        FS_DirectoryEntry   entry;
        Result              res;
        u32                 units;
        u32                 entriesNb = 0;
        u8                  filename[PATH_MAX + 1] = {0};

        if (!_isListed)
            _List();

        for (int i = 0; i < _list.size(); i++)
        {
            entry = _list[i];

            if (!(entry.attributes & 1))
                continue;
            std::memset(filename, 0, sizeof(filename));
            units = utf16_to_utf8(filename, entry.name, PATH_MAX);
            if (units < 0)
                continue;
            std::string fn = (char *)filename;
            if (patternCheck && fn.find(pattern) == std::string::npos)
                continue;
            folders.push_back(fn);  
        }
        return (0);
    }
}
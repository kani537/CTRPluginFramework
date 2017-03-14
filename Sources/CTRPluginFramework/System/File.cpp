#include "CTRPluginFramework/System/Directory.hpp"
#include "CTRPluginFramework/System/File.hpp"
#include "CTRPluginFramework/System/Process.hpp"

#include "ctrulib/util/utf.h"
#include "ctrulib/result.h"
#include <limits.h>
#include <cstring>

namespace CTRPluginFramework
{
    extern FS_Archive   _sdmcArchive;

    int     File::Create(std::string path)
    {
        FS_Path fsPath = Directory::_SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (-1);

        Result res;

        res = FSUSER_CreateFile(_sdmcArchive, fsPath, 0, 0);
        if (R_SUCCEEDED(res))
            return (0);
        return (res);
    }

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

    int     File::Exists(std::string path)
    {
        FS_Path fsPath = Directory::_SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (-1);

        Result res;
        Handle handle;

        res = FSUSER_OpenFile(&handle, _sdmcArchive, fsPath, FS_OPEN_READ, 0);
        if (R_SUCCEEDED(res))
        {
            FSFILE_Close(handle);
            return (1);
        }
        return (0);
    }

    int     File::Open(File &output, std::string path, int mode)
    {
        FS_Path fsPath = Directory::_SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (-1);

        int fsmode = mode & 0b111;
        Handle handle;
        Result res;

        res = FSUSER_OpenFile(&handle, _sdmcArchive, fsPath, fsmode, 0);
        /*if (R_FAILED(res) && mode & CREATE)
        {
            res = Create(path);
            if (R_SUCCEEDED(res))  
            {
                res = FSUSER_OpenFile(&handle, _sdmcArchive, fsPath, fsmode, 0);
            } 
                
        }*/
        if (R_FAILED(res))
            return (res);

        output._handle = handle;
        output._mode = mode;
        output._path = path;
        output._offset = 0;

        if (mode & TRUNCATE)
            FSFILE_SetSize(handle, 0);

        u32     pos = path.rfind('/');

        if (pos != std::string::npos)
            path = path.substr(pos);

        pos = path.rfind('.');
        if (pos != std::string::npos)
            output._name = path.substr(0, pos);
        else
            output._name = path;
        //output = File(path, handle, mode);
        return (res);

    }

    File::File(std::string &path, Handle handle, int mode) :
    _path(path), _mode(mode), _offset(0)
    {
        _handle = handle;
        if (mode & TRUNCATE)
            FSFILE_SetSize(handle, 0);

        u32     pos = path.rfind('/');

        if (pos != std::string::npos)
            path = path.substr(pos);

        pos = path.rfind('.');
        if (pos != std::string::npos)
            _name = path.substr(0, pos);
        else
            _name = path;
    }

    int     File::Close(void)
    {
        return (FSFILE_Close(_handle));
    }

    int     File::Read(void *buffer, u32 length)
    {
        if (!buffer)
            return (-1);
        if (length == 0)
            return (0);

        Result  res;
        u32     bytes;

        res = FSFILE_Read(_handle, &bytes, _offset, (u32 *)buffer, length);
        if (R_FAILED(res))
            return (res);
        _offset += bytes;
        return (0);
    }

    int     File::Write(const void *data, u32 length)
    {
        if (!data || !(_mode & WRITE))
            return (-1);
        if (length == 0)
            return (0);

        Result  res;
        u32     bytes;
        u32     sync = FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME;

        if (_mode & APPEND)
            FSFILE_GetSize(_handle, &_offset);

        res = FSFILE_Write(_handle, &bytes, _offset, (u32 *)data, length, sync);
        if (R_FAILED(res))
            return (res);

        _offset += bytes;
        return (0);
    }

    int     File::WriteLine(std::string line)
    {
        line += "\r\n";
        return (Write(line.c_str(), line.size()));
    }

    int     File::Seek(s64 position, SeekPos rel)
    {
        u64 offset;

        switch (rel)
        {
            case SET:
            {
                offset = 0;
                break;
            }
            case CUR:
            {
                offset = _offset;
                break;
            }
            case END:
            {
                if (R_FAILED(FSFILE_GetSize(_handle, &offset)))
                    return (-1);
                break;
            }
            default:
            {
                return (-1);
            }
        }

        if (position < 0 && offset < -position)
            return (-1);

        _offset = offset + position;
        return (0);
    }

    u64     File::Tell(void)
    {
        return (_offset);
    }

    void    File::Rewind(void)
    {
        _offset = 0;
    }

    u64     File::GetSize(void)
    {
        u64 file = 0;

        if (R_FAILED(FSFILE_GetSize(_handle, &file)))
            return (-1);
        return (file);
    }

    int     File::Dump(u32 address, u32 length)
    {
        if (!Process::ProtectMemory(address, length))
            return (-1);
        svcFlushProcessDataCache(Process::GetHandle(), (void *)address, length);
        return (Write((void *)address, length));
    }

    int     File::Inject(u32 address, u32 length)
    {
        u8  *buffer = new u8[0x1001];

        if (buffer == nullptr || !Process::ProtectMemory(address, length))
            return (-1);

        do
        {
            int size = length <= 0x1000 ? length : 0x1000;
            // First read file
            if (Read(buffer, size) != 0)
                goto clean;
            // Then patch process
            if (!Process::Patch(address, buffer, size))
                goto clean;

            // refresh data
            address += size;
            length -= size;

        } while (length > 0);

        return (0);
    clean:
        delete[] buffer;
        return (-1);
    }
}
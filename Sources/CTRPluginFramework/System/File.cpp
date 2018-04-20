#include "CTRPluginFramework/System/File.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "CTRPluginFrameworkImpl/System/ProcessImpl.hpp"
#include "ctrulib/services/fs.h"
#include "ctrulib/util/utf.h"
#include "ctrulib/result.h"
#include <limits.h>
#include <cstring>

namespace CTRPluginFramework
{
    extern FS_Archive   _sdmcArchive;

    namespace _Path {
        int     SdmcFixPath(std::string &path);
        FS_Path SdmcUtf16Path(std::string path);
    }

    int     File::Create(const std::string &path)
    {
        FS_Path fsPath = _Path::SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (INVALID_PATH);

        Result res;

        res = FSUSER_CreateFile(_sdmcArchive, fsPath, 0, 0);
        if (R_SUCCEEDED(res))
            return (SUCCESS);
        return (res);
    }

    int     File::Rename(const std::string &oldPath, const std::string &newPath)
    {
        FS_Path oldFsPath = _Path::SdmcUtf16Path(oldPath);
        FS_Path newFsPath;
        uint16_t    path[PATH_MAX + 1] = {0};

        if (oldFsPath.data == nullptr)
            return (INVALID_PATH);

        std::memcpy(path, oldFsPath.data, PATH_MAX);
        oldFsPath.data = (u8 *)path;

        newFsPath = _Path::SdmcUtf16Path(newPath);
        if (newFsPath.data == nullptr)
            return (INVALID_PATH);

        Result res;

        res = FSUSER_RenameFile(_sdmcArchive, oldFsPath, _sdmcArchive, newFsPath);
        if (R_SUCCEEDED(res))
            return (SUCCESS);

        return (res);
    }

    int     File::Remove(const std::string &path)
    {
        FS_Path fsPath = _Path::SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (INVALID_PATH);

        Result res;

        res = FSUSER_DeleteFile(_sdmcArchive, fsPath);
        if (R_SUCCEEDED(res))
            return (SUCCESS);

        return (res);
    }

    int     File::Exists(const std::string &path)
    {
        FS_Path fsPath = _Path::SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (INVALID_PATH);

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

    int     File::Open(File &output, const std::string &path, int mode)
    {
        if (output._isOpen)
            output.Close();

        FS_Path fsPath = _Path::SdmcUtf16Path(path);

        if (fsPath.data == nullptr)
            return (INVALID_PATH);

        int fsmode = mode & 0b111;
        Handle handle;
        Result res;

        // So we can get path even if the opening failed
        output._path = path;
        _Path::SdmcFixPath(output._path);

        res = FSUSER_OpenFile(&handle, _sdmcArchive, fsPath, fsmode, 0);

        if (R_FAILED(res))
            return (res);

        output._handle = handle;
        output._mode = mode;
        output._offset = 0;
        output._isOpen = true;

        if (mode & TRUNCATE)
            FSFILE_SetSize(handle, 0);
        return (res);
    }

    int     File::Close(void) const
    {
        if (!_isOpen)
            return (NOT_OPEN);

        Result res = FSFILE_Close(_handle);

        if (R_SUCCEEDED(res))
        {
            _path = "";
            _handle = 0;
            _offset = 0;
            _mode = 0;
            _isOpen = false;
        }

        return (res);
    }

    int     File::Read(void *buffer, u32 length) const
    {
        if (!_isOpen) return (NOT_OPEN);
        if (!(_mode & READ)) return (INVALID_MODE);
        if (!buffer || !Process::CheckAddress((u32)buffer, MEMPERM_WRITE)) return (INVALID_ARG);
        if (length == 0) return (SUCCESS);

        Result  res;
        u32     bytes;

        res = FSFILE_Read(_handle, &bytes, _offset, (u32 *)buffer, length);
        if (R_FAILED(res))
            return (res);
        _offset += bytes;
        return (SUCCESS);
    }

    int     File::Write(const void *data, u32 length)
    {
        if (!_isOpen) return (NOT_OPEN);
        if (!(_mode & WRITE)) return (INVALID_MODE);
        if (!data || !Process::CheckAddress((u32)data, MEMPERM_READ)) return (INVALID_ARG);
        if (length == 0) return (SUCCESS);

        Result  res = 0;
        u32     bytes;
        u32     sync = _mode & SYNC ?  FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME : 0;

        if (_mode & APPEND)
            res = FSFILE_GetSize(_handle, &_offset);

        if (R_FAILED(res))
            return (res);

        res = FSFILE_Write(_handle, &bytes, _offset, (u32 *)data, length, sync);
        if (R_FAILED(res))
            return (res);

        _offset += bytes;
        return (SUCCESS);
    }

    int     File::WriteLine(std::string line)
    {
        line += "\r\n";
        return (Write(line.c_str(), line.size()));
    }

    int     File::Seek(s64 position, SeekPos rel) const
    {
        if (!_isOpen)
            return (NOT_OPEN);

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
                Result res = FSFILE_GetSize(_handle, &offset);
                if (R_FAILED(res))
                    return (res);
                break;
            }
            default:
            {
                return (INVALID_ARG);
            }
        }

        if (position < 0 && offset < -position)
            return (INVALID_ARG);

        _offset = offset + position;
        return (SUCCESS);
    }

    u64     File::Tell(void) const
    {
        return (_offset);
    }

    void    File::Rewind(void) const
    {
        _offset = 0;
    }

    int     File::Flush(void) const
    {
        return (FSFILE_Flush(_handle));
    }

    u64     File::GetSize(void) const
    {
        if (!_isOpen)
            return (NOT_OPEN);

        u64 size = 0;
        Result res = FSFILE_GetSize(_handle, &size);
        if (R_FAILED(res))
            return (res);
        return (size);
    }

    int     File::Dump(u32 address, u32 length)
    {
        if (!_isOpen) return (NOT_OPEN);
        if (!(_mode & WRITE)) return (INVALID_MODE);
        if (!Process::CheckAddress(address, MEMPERM_READ))
            return(INVALID_ARG);

        bool    unpause = false;
        Result  res = SUCCESS;

        // If game not paused, then pause it
        if (!ProcessImpl::IsPaused())
        {
            ProcessImpl::Pause(false);
            unpause = true;
        }

        svcFlushProcessDataCache(Process::GetHandle(), (void *)address, length);

        // Dump in chunk of 32kb
        do
        {
            int size = length > 0x8000 ? 0x8000 : length;
            res = Write((void *)address, size);

            address += size;
            length -= size;

        } while (length > 0 && R_SUCCEEDED(res));

        if (unpause)
            ProcessImpl::Play(false);
        return (res);
    }

    int     File::Inject(u32 address, u32 length) const
    {
        if (!_isOpen) return (NOT_OPEN);
        if (!(_mode & READ)) return (INVALID_MODE);
        if (!Process::CheckAddress(address)) return (INVALID_ARG);

        u8      *buffer = ::new u8[0x40000]; //256KB


        if (buffer == nullptr)
            return (MAKERESULT(RL_USAGE, RS_OUTOFRESOURCE, 0, RD_OUT_OF_MEMORY)); ///< I think it'll abort() so it won't be reached anyway

        bool    unpause = false;
        Result  res = SUCCESS;

        // If game not paused, then pause it
        if (!ProcessImpl::IsPaused())
        {
            ProcessImpl::Pause(false);
            unpause = true;
        }

        do
        {
            int size = length <= 0x40000 ? length : 0x40000;
            // First read file
            res = Read(buffer, size);
            if (R_FAILED(res))
                break;

            // Then patch process
            if (!Process::Patch(address, buffer, size))
                res = UNEXPECTED_ERROR;

            // refresh data
            address += size;
            length -= size;

        } while (length > 0 && res == SUCCESS);

        delete[] buffer;
        if (unpause)
            ProcessImpl::Play(false);
        return (res);
    }

    bool    File::IsOpen(void) const
    {
        return (_isOpen);
    }

    std::string     File::GetFullName(void) const
    {
        return (_path);
    }

    std::string     File::GetName(void) const
    {
        int pos = _path.rfind("/");

        if (pos != std::string::npos)
            return (_path.substr(pos + 1));
        return (_path); ///< Better return the full path than nothing
    }

    std::string     File::GetExtension(void) const
    {
        int pos = _path.rfind(".");

        if (pos != std::string::npos)
            return (_path.substr(pos + 1));
        return (std::string());
    }

    File::File(void): _handle(0), _offset(0), _mode(0), _isOpen(false)
    {
    }

    Handle __fileHandle;
    File::File(const std::string& path, u32 mode) :
        _handle(0), _offset(0), _mode(0), _isOpen(false)
    {
        Open(*this, path, mode);
        __fileHandle = _handle;
    }
}

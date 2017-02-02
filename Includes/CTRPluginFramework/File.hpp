#ifndef CTRPLUGINFRAMEWORK_FILE_HPP
#define CTRPLUGINFRAMEWORK_FILE_HPP

#include "types.h"
#include "ctrulib/services/fs.h"

#include <string>
#include <vector>

namespace CTRPluginFramework
{
    class File
    {
    public:

        enum SeekPos
        {
            CUR,
            SET,
            END
        };

        enum Mode
        {
            READ = 1,
            WRITE = 1 << 1,
            CREATE = 1 << 2,
            APPEND = 1 << 3,
            TRUNCATE = 1 << 4
        };

        static int  Create(std::string path);
        static int  Rename(std::string path, std::string newName);
        static int  Remove(std::string path);
        static int  IsExists(std::string path);
        static int  Open(File &output, std::string path, int mode = READ | WRITE | CREATE);
        
        ~File(){ Close(); }

        int     Close(void);

        int     Read(void *buffer, u32 length);
        int     Write(const void *data, u32 length);
        int     WriteLine(std::string line);
        int     Seek(s64 offset, SeekPos rel = CUR);
        u64     GetSize(void);

        int     Dump(u32 address, u32 length);
        int     Inject(u32 address, u32 length);


    private:
        std::string     _path;
        std::string     _name;
        Handle          _handle;
        u64             _offset;
        int             _mode;

        File (std::string &path, Handle &handle, int mode);
    };
}

#endif
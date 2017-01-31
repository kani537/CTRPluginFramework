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

        enum Seek
        {
            CUR,
            BEGIN,
            END
        };

        enum Mode
        {
            READ = 1,
            WRITE = 1 << 1,
            CREATE = 1 << 2
        };

        static int  Rename(std::string &path, std::string &newName);
        static int  Remove(std::string &path);
        static bool IsExists(std::string &path);
        static int  Open(File &output, std::string &path, int mode = READ | WRITE | CREATE);
        
        ~File(){ Close(); }

        int     Close(void);

        int     Read(void *buffer, u32 length);
        int     Write(const void *data, u32 length);
        int     WriteLine(const std::string &line);
        int     WriteLine(const char *line);
        int     Seek(int offset, Seek rel = CUR);

        int     Dump(u32 address, u32 length);
        int     Inject(u32 address, u32 length);


    private:
        FS_Path         _fspath;
        std::string     _path;
        std::string     _name;
        Handle          _handle;
        u32             _offset;
        int             _mode;

        File (std::string &path, Handle &handle);
    };
}

#endif
#include "CTRPluginFramework/Utils/LineReader.hpp"
#include "CTRPluginFramework/System/File.hpp"
#include <algorithm>

namespace CTRPluginFramework
{
    LineReader::LineReader(File &file) :
        _file(file),
        _offsetInBuffer(0),
        _dataInBuffer(0),
        _buffer(new char[0x1000])
    {

    }

    LineReader::~LineReader(void)
    {
        delete[] _buffer;
    }

    bool    LineReader::operator()(std::string &line)
    {
        line.clear();

        u64     fileSize = _file.GetSize();

        do
        {
            u64     leftSize = std::max((s64)(fileSize - _file.Tell()), (s64)0);

            // EOF
            if (!leftSize && _offsetInBuffer >= _dataInBuffer)
                return (!line.empty());

            // If there's data in buffer
            if (_offsetInBuffer < _dataInBuffer)
            {
                while (_offsetInBuffer < _dataInBuffer)
                {
                    char c = _buffer[_offsetInBuffer++];

                    if (c == '\n')
                        return (true);

                    if (c && c != '\r')
                        line += c;
                }
                continue;
            }

            // Read file
            u32     size = (u32)(std::min((u64)0x1000, leftSize));

            if (_file.Read(_buffer, size)) goto error;

            _dataInBuffer = size;
            _offsetInBuffer = 0;

        } while (_dataInBuffer);

    error:
        _dataInBuffer = 0;
        _offsetInBuffer = 0;
        return (false);
    }
}

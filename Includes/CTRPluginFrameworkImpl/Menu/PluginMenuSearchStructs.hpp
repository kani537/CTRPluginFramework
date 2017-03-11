#ifndef CTRPLUGINFRAMEWORKIMPL_SEARCHSTRUCT_HPP
#define CTRPLUGINFRAMEWORKIMPL_SEARCHSTRUCT_HPP

#include "types.h"

#include "CTRPluginFramework/System/File.hpp"
#include <vector>

namespace CTRPluginFramework
{
    enum class SearchSize
    {
        Bits8 = 0,
        Bits16,
        Bits32,
        Bits64,
        FloatingPoint,
        Double
    };

    enum class  SearchType
    {
        ExactValue = 0,
        Unknown = 1
    };

    enum class CompareType
    {
        Equal = 0,
        NotEqual,
        GreaterThan,
        GreaterOrEqual,
        LesserThan,
        LesserOrEqual
    };

    template<typename T>
    struct SearchResult
    {
        u32     address;
        T       value;
    };

    /*template <typename T>
    inline int ReadFromFile(File &file, T &t)
    {
        return (file.Read(reinterpret_cast<char *>(&t), sizeof(T)));
    }

    template <typename T>
    inline int WriteToFile(File &file, const T &t) const
    {
        return (file.Write(reinterpret_cast<const char *>(&t), sizeof(T)));
    }*/

    struct SearchBase
    {
        u32     start; //<- Start address
        u32     end; //<- End address
        u32     currentPosition; //<- Current position in the search range
        u32     alignment; //<- search alignment
        u32     resultCount; //<- results counts

        SearchType  type;
        SearchSize  size;
        CompareType compare;

        File    file; //<- File associated for read / write
    };

    template<typename T>
    struct Search : SearchBase
    {
        T       value; //<- Value to find       

        std::vector<SearchResult<T>> results; //<- Hold the results     
    };
}

#endif
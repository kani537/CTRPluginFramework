#ifndef CTRPLUGINFRAMEWORKIMPL_SEARCHSTRUCT_HPP
#define CTRPLUGINFRAMEWORKIMPL_SEARCHSTRUCT_HPP

#include "types.h"

#include "CTRPluginFramework/System/File.hpp"
#include <vector>

namespace CTRPluginFramework
{
    enum class SearchSize : u8
    {
        Bits8 = 0,
        Bits16,
        Bits32,
        Bits64,
        FloatingPoint,
        Double
    };

    enum class  SearchType : u8
    {
        ExactValue = 0,
        Unknown = 1
    };

    enum class CompareType : u8
    {
        Equal = 0,
        NotEqual,
        GreaterThan,
        GreaterOrEqual, 
        LesserThan,
        LesserOrEqual,
        DifferentBy,
        DifferentByLess,
        DifferentByMore
    };

    template<typename T>
    struct SearchResult
    {
        SearchResult(u32 addr, T v) : address(addr), value(v) {}

        u32     address;
        T       value;
    };

    class   SearchBase
    {
    public:

        SearchBase(u32 start, u32 end, u32 alignment, SearchBase *prev);
        ~SearchBase(){}

        virtual bool    DoSearch(void) = 0;
        virtual bool    ResultsToFile(void) = 0;
        virtual u32     GetHeaderSize(void) = 0;

        SearchType      Type; 
        SearchSize      Size;
        CompareType     Compare;

        u32             ResultCount; //<- results counts
        float           Progress; //<- current progression of the search in %
    private: 
        u32     _step; //<- current step
        u32     _startRange; //<- Start address
        u32     _endRange; //<- End address
        u32     _currentPosition; //<- Current position in the search range
        u8      _alignment; //<- search alignment
        

        File    _file; //<- File associated for read / Write

        SearchBase *_previousSearch; 

    };

    template<typename T>
    class Search : SearchBase
    {
        using ResultIter = typename std::vector<SearchResult<T>>::Iterator;

    public:

        Search(T value, u32 start, u32 end, u32 alignment, SearchBase *previous);

        /*
        ** DoSearch
        ** Override SearchBase's
        ** Return if search is done
        ****************************/
        bool    DoSearch(void);

        /*
        ** ResultsToFile
        ** Override SearchBase's
        ** Return if search is done
        ****************************/
        bool    ResultsToFile(void);

    private:

        std::vector<SearchResult<T>>    _results; //<- Hold the results
        T                               _checkValue; //<- Value to compare with

        /*
        ** Methods
        ***********/

        void        _FirstExactSearch(u32 &start, u32 end, u32 maxResult);
        // Return true if the value matches the search settings
        bool        _Compare(T old, T newer);
        
        bool        _WriteHeaderToFile(void);
        u32         _GetHeaderSize(void);
        u32         _GetResultStructSize(void);
        
        bool        _ReadResults(std::vector<SearchResult<T>> &out, u32 index, u32 count);

        inline int ReadFromFile(File &file, T &t)
        {
            return (file.Read(reinterpret_cast<char *>(&t), sizeof(T)));
        }

        inline int WriteToFile(File &file, const T &t) const
        {
            return (file.Write(reinterpret_cast<const char *>(&t), sizeof(T)));
        }

    };
}

#endif
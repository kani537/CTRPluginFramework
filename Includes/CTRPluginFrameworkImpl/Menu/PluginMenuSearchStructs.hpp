#ifndef CTRPLUGINFRAMEWORKIMPL_SEARCHSTRUCT_HPP
#define CTRPLUGINFRAMEWORKIMPL_SEARCHSTRUCT_HPP

#include "types.h"

#include "CTRPluginFramework/System/File.hpp"
#include "CTRPluginFramework/System/Clock.hpp"
#include <vector>
#include <iterator>

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

    template <typename T>
    struct SearchResult
    {
        SearchResult() : address(0), value(0) {}
        SearchResult(u32 addr, T v) : address(addr), value(v) {}

        u32     address;
        T       value;
    };

    class   SearchBase
    {
        using stringvector = std::vector<std::string>;

    public:

        SearchBase(u32 start, u32 end, u32 alignment, SearchBase *prev);
        ~SearchBase(){}

        virtual void    Cancel(void) = 0;
        virtual bool    DoSearch(void) = 0;
        virtual bool    ResultsToFile(void) = 0;
        virtual bool    FetchResults(stringvector &address, stringvector &newval, stringvector &oldvalue) = 0;
        //virtual u32     GetHeaderSize(void) = 0;

        SearchType      Type; 
        SearchSize      Size;
        CompareType     Compare;

        u32             ResultCount; //<- results counts
        float           Progress; //<- current progression of the search in %
        Time            SearchTime;
    protected: 
        friend class SearchMenu;
        u32     _step; //<- current step
        u32     _startRange; //<- Start address
        u32     _endRange; //<- End address
        u32     _currentPosition; //<- Current position in the search range
        u8      _alignment; //<- search alignment
        Clock   _clock;
        

        File    _file; //<- File associated for read / Write

        SearchBase *_previousSearch; 

    };
    
    //template <typename T>
    

    template <typename T>
    class Search : public SearchBase
    {
        using ResultIter = typename std::vector<SearchResult<T>>::iterator;
        using stringvector = std::vector<std::string>;
        
        using CmpFunc = bool (Search<T>::*)(T, T);

    public:

        Search(T value, u32 start, u32 end, u32 alignment, SearchBase *previous);

        /*
        ** Cancel
        */
        void    Cancel(void);
        /*
        ** DoSearch
        ** Override SearchBase's
        ** Return true if search is done
        ****************************/
        bool    DoSearch(void);

        /*
        ** ResultsToFile
        ** Override SearchBase's
        ** Return true if suceeded
        ****************************/
        bool    ResultsToFile(void);

        bool    FetchResults(stringvector &address, stringvector &newval, stringvector &oldvalue);

    private:

        //std::vector<SearchResult<T>>    _results; //<- Hold the results
        T                               _checkValue; //<- Value to compare with
        u32                             _maxResult;
        SearchResult<T>                 *_resultsArray;
        SearchResult<T>                 *_resultsEnd;
        SearchResult<T>                 *_resultsP;
        CmpFunc                         _compare;

        /*
        ** Methods
        ***********/

        void        _FirstExactSearch(u32 &start, u32 end, u32 maxResult);
        void        _UpdateCompare(void);
        // Return true if the value matches the search settings
        //bool        _Compare(T old, T newer);

        bool        _CompareE(T old, T newer);
        bool        _CompareNE(T old, T newer);
        bool        _CompareGT(T old, T newer);
        bool        _CompareGE(T old, T newer);
        bool        _CompareLT(T old, T newer);
        bool        _CompareLE(T old, T newer);
        bool        _CompareDB(T old, T newer);
        bool        _CompareDBL(T old, T newer);
        bool        _CompareDBM(T old, T newer);

        bool        _CompareUnknownE(T old, T newer);
        bool        _CompareUnknownNE(T old, T newer);
        bool        _CompareUnknownGT(T old, T newer);
        bool        _CompareUnknownGE(T old, T newer);
        bool        _CompareUnknownLT(T old, T newer);
        bool        _CompareUnknownLE(T old, T newer);
        
        bool        _WriteHeaderToFile(void);
        u32         _GetHeaderSize(void);
        u32         _GetResultStructSize(void);
        
        bool        _ReadResults(std::vector<SearchResult<T>> &out, u32 index, u32 count);

        template <typename U>
        inline int ReadFromFile(File &file, U &t)
        {
            return (file.Read(reinterpret_cast<char *>(&t), sizeof(U)));
        }

        template <typename U>
        inline int WriteToFile(File &file, const U &t) const
        {
            return (file.Write(reinterpret_cast<const char *>(&t), sizeof(U)));
        }

    };
}

#endif
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

    private:

        u32     _step; //<- current step in the search process
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
        /*
        ** Members
        ***********/
        T       checkValue; //<- Value to compare with
        std::vector<SearchResult<T>> results; //<- Hold the results

        /*
        ** Methods
        ***********/
        void    FirstExactSearch(u32 &start, u32 end, u32 maxResult);
        bool    Compare(T old, T newer);


        /*
        ** Search
        ** Return if search is done
        ****************************/
        bool    DoSearch(void)
        {
            // First Search ?
            if (previousSearch == nullptr)
            {
                u32 start = currentPosition >= endRange ? startRange : currentPosition;
                u32 maxResult = 0x1000 / GetResultStructSize();
                u32 end = start + std::min((u32)(endRange - start), (u32)0x1000);

                if (type == SearchType::ExactValue)
                {
                    FirstExactSearch(start, end, maxResult);
                }                
                else // Unknown value
                {
                    for (; start < end; start += alignment)
                    {
                        T v = *(static_cast<T *>(start));

                        resultCount++;
                        results.push_back(SearchResult<T>(start, v));
                        if (results.size() >= maxResult)
                            break;
                    }

                    // Update position
                    currentPosition = start;
                }
            }
            // Second search
            else
            {
                
                std::vector<SearchResult<T>>    oldResults;

                u32 maxResult = 0x1000 / GetResultStructSize();

                // First get the results from the file
                if (ReadResults(oldResults, currentPosition, maxResult))
                {
                    ResultIter iter = oldResults.begin();
                    ResultIter end = oldResults.end();

                    for (; iter != end; iter++)
                    {
                        T val = *static_cast<T *>(iter.address);
                        T oldVal = iter.value;
                        if (Compare(oldVal, val, checkValue))
                        {
                            resultCount++;
                            results.push_back(SearchResult<T>(iter.address, val));
                        }
                    }

                    // Update position
                    currentPosition += oldResults.size();
                }



            }

            if (currentPosition >= endRange)
                return (true);

            return (false);
        }

        inline int ReadFromFile(File &file, T &t)
        {
            return (file.Read(reinterpret_cast<char *>(&t), sizeof(T)));
        }

        inline int WriteToFile(File &file, const T &t) const
        {
            return (file.Write(reinterpret_cast<const char *>(&t), sizeof(T)));
        }

        bool    WriteHeaderToFile(void)
        {
            u64     offset = file.Tell();
            int     ret = 0;

            file.Rewind();

            ret |= WriteToFile(file, type);
            ret |= WriteToFile(file, compare);
            ret |= WriteToFile(file, startRange);
            ret |= WriteToFile(file, endRange);
            ret |= WriteToFile(file, alignment);
            ret |= WriteToFile(file, resultCount);
            ret |= WriteToFile(file, size);
            ret |= WriteToFile(file, sizeof(T));
            ret |= WriteToFile(file, GetHeaderSize());
            u32 size = sizeof(u32) + sizeof(T);
            ret |= WriteToFile(file, size);
            ret |= WriteToFile(file, checkValue);

            if (offset != 0)
                file.Seek(offset, File::SeekPos::SET);

            if (ret != 0)
                return (false);

            return (true);
        }

        u32     GetHeaderSize(void)
        {
            return 
            (  
                sizeof(type)
                + sizeof(compare)
                + sizeof(startRange)
                + sizeof(endRange)
                + sizeof(alignment)
                + sizeof(resultCount)
                + sizeof(size)
                + sizeof(T)
                + sizeof(u32) // offset in file where results starts
                + sizeof(u32) // SearchResultSize
                + sizeof(T) // value compared with
            );
        }

        u32     GetResultStructSize(void)
        {
            return 
            (
                sizeof(u32) // address
                + sizeof(T)
            );
        }

        bool    ResultsToFile(void)
        {
            // Write Header
            WriteHeaderToFile();

            // Write all results
            file.Write(results.data(), results.size() * GetResultStructSize());
        }

        bool    ReadResults(std::vector<SearchResult<T>> &out, u32 index, u32 count)
        {
            if (index > resultCount)
                return (false);

            if (index + count > resultCount)
                count = resultCount - index;

            u64 offset = GetHeaderSize() + GetResultStructSize() * index;

            // Go to the file offset
            file.Seek(offset, File::SeekPos::SET);

            // Reserve memory and create default object
            out.Resize(count);

            //Read results
            if (file.Read((void *)out.data(), count * GetResultStructSize()) != 0)
                return (true);

            return (false);
        }

    };
}

#endif
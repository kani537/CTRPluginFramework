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
        LesserOrEqual
    };

    template<typename T>
    struct SearchResult
    {
        SearchResult(u32 addr, T v) : address(addr), value(v) {}

        u32     address;
        T       value;
    };

    struct SearchBase
    {
        u32     startRange; //<- Start address
        u32     endRange; //<- End address
        u32     currentPosition; //<- Current position in the search range
        u8      alignment; //<- search alignment
        u32     resultCount; //<- results counts

        SearchType  type;
        SearchSize  size;
        CompareType compare;

        File    file; //<- File associated for read / Write

        virtual bool DoSearch(void) = 0;
        virtual bool ResultsToFile(void) = 0;
        virtual u32  GetHeaderSize(void) = 0;   

    };

    template<typename T>
    struct Search : SearchBase
    {
        T       value; //<- Value to find       

        std::vector<SearchResult<T>> results; //<- Hold the results

        // Return if search is done
        bool    DoSearch(void)
        {
            u32 start = currentPosition >= endRange ? startRange : currentPosition;
            u32 end = start + std::min((u32)(endRange - start), (u32)0x1000);

            if (type == SearchType::ExactValue)
            {
                switch (compare)
                {
                    case CompareType::Equal:
                    {
                        for (; start <= end; start += alignment)
                        {
                            T v = *(static_cast<T *>(start));
                            if (v == value)
                            {
                                resultCount++;
                                results.push_back(SearchResult<T>(start, v));
                            }
                        }
                        break;
                    }
                    case CompareType::NotEqual:
                    {
                        for (; start <= end; start += alignment)
                        {
                            T v = *(static_cast<T *>(start));
                            if (v != value)
                            {
                                resultCount++;
                                results.push_back(SearchResult<T>(start, v));
                            }
                        }
                        break;
                    }
                    case CompareType::GreaterThan:
                    {
                        for (; start <= end; start += alignment)
                        {
                            T v = *(static_cast<T *>(start));
                            if (v > value)
                            {
                                resultCount++;
                                results.push_back(SearchResult<T>(start, v));
                            }
                        }
                        break;
                    }
                    case CompareType::GreaterOrEqual:
                    {
                        for (; start <= end; start += alignment)
                        {
                            T v = *(static_cast<T *>(start));
                            if (v >= value)
                            {
                                resultCount++;
                                results.push_back(SearchResult<T>(start, v));
                            }
                        }
                        break;
                    }
                    case CompareType::LesserThan:
                    {
                        for (; start <= end; start += alignment)
                        {
                            T v = *(static_cast<T *>(start));
                            if (v < value)
                            {
                                resultCount++;
                                results.push_back(SearchResult<T>(start, v));
                            }
                        }
                        break;
                    }
                    case CompareType::LesserOrEqual:
                    {
                        for (; start <= end; start += alignment)
                        {   
                            T v = *(static_cast<T *>(start));
                            if (v <= value)
                            {
                                resultCount++;
                                results.push_back(SearchResult<T>(start, v));
                            }
                        }
                        break;
                    }
                } // End switch
            }
            // Unknown value
            else
            {
                return (true);
            }

            currentPosition = start;

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
            ret |= WriteToFile(file, value);

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
            );
        }

        bool    ResultsToFile(void)
        {

        }

        bool    ReadResultsFromFile(std::vector<SearchResult<T>> &output, u32 size)
        {

        }
    };
}

#endif
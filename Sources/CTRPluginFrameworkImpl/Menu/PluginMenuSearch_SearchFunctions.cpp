#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearchStructs.hpp"
#include "CTRPluginFramework/System/Process.hpp"

namespace CTRPluginFramework
{
    template <typename T>
    void      Search<T>::FirstExactSearch(u32 &start, u32 end, u32 maxResult)
    {
        switch (compare)
        {
            case CompareType::Equal:
            {
                for (; start < end; start += alignment)
                {
                    T v = *(static_cast<T *>(start));
                    if (v == checkValue)
                    {
                        resultCount++;
                        results.push_back(SearchResult<T>(start, v));
                        if (results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            case CompareType::NotEqual:
            {
                for (; start < end; start += alignment)
                {
                    T v = *(static_cast<T *>(start));
                    if (v != checkValue)
                    {
                        resultCount++;
                        results.push_back(SearchResult<T>(start, v));
                        if (results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            case CompareType::GreaterThan:
            {
                for (; start < end; start += alignment)
                {
                    T v = *(static_cast<T *>(start));
                    if (v > checkValue)
                    {
                        resultCount++;
                        results.push_back(SearchResult<T>(start, v));
                        if (results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            case CompareType::GreaterOrEqual:
            {
                for (; start < end; start += alignment)
                {
                    T v = *(static_cast<T *>(start));
                    if (v >= checkValue)
                    {
                        resultCount++;
                        results.push_back(SearchResult<T>(start, v));
                        if (results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            case CompareType::LesserThan:
            {
                for (; start < end; start += alignment)
                {
                    T v = *(static_cast<T *>(start));
                    if (v < checkValue)
                    {
                        resultCount++;
                        results.push_back(SearchResult<T>(start, v));
                        if (results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            case CompareType::LesserOrEqual:
            {
                for (; start < end; start += alignment)
                {   
                    T v = *(static_cast<T *>(start));
                    if (v <= checkValue)
                    {
                        resultCount++;
                        results.push_back(SearchResult<T>(start, v));
                        if (results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            default:
                break;
        } // End switch
    }

    // Return if it matches the condition   
    template<typename T>
    bool    Search<T>::Compare(T older, T newer)
    {
        switch (compare)
        {
            case CompareType::Equal: return (checkValue == newer);
            case CompareType::NotEqual: return (checkValue != newer);
            case CompareType::GreaterThan: return (newer > checkValue);
            case CompareType::GreaterOrEqual: return (newer >= checkValue);
            case CompareType::LesserThan: return (newer < checkValue);
            case CompareType::LesserOrEqual: return (newer <= checkValue);
            case CompareType::DifferentBy: return (newer == (older + checkValue) || newer == (older - checkValue));
            case CompareType::DifferentByLess: return ((older - checkValue) < newer && newer < (older + checkValue));
            case CompareType::DifferentByMore: return (newer < (older - checkValue) || newer > (older + checkValue));
            default: return (false);
        }
    }
}
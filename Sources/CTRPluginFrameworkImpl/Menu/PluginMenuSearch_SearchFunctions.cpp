#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearchStructs.hpp"
#include "CTRPluginFramework/System/Directory.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include <ctime.hpp>

#include <string>

namespace CTRPluginFramework
{

    /*
    ** SearchBase
    *******************/

    SearchBase::SearchBase(u32 start, u32 end, u32 alignement, SearchBase *previous) :
    _startRange(start), _endRange(end), _alignment(alignement), _previousSearch(previous)
    {
        // Init search variables
        _currentPosition = 0;
        resultCount = 0;

        // Default search parameters
        Type = SearchType::ExactValue;
        Size = SearchSize::Bits32;
        Compare = CompareType::Equal;

        _step = _previousSearch == nullptr ? 0 : _previousSearch->_step + 1;

        // Open file
        char    tid[17] = {0};
        Process::GetTitleID(tid);
        std::string path = tid + "-Step" + to_string(_step) + ".bin";


        // Open current directory
        Directory dir;
        if (Directory::Open(dir, "Search", true) == 0)
        {
            // Open file
            File::Open(_file, path, File::READ | File::WRITE | File::CREATE);
        }
    }

    /*
    ** Search : SearchBase
    ************/

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
#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearchStructs.hpp"
#include "CTRPluginFramework/System/Directory.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include <time.h>

#include <string>

namespace CTRPluginFramework
{

    /*
    ** SearchBase
    *******************/

    SearchBase::SearchBase(u32 start, u32 end, u32 alignment, SearchBase *previous) :
    _startRange(start), _endRange(end), _alignment(alignment), _previousSearch(previous), Progress(0.0f)
    {
        // Init search variables
        _currentPosition = 0;
        ResultCount = 0;

        // Default search parameters
        Type = SearchType::ExactValue;
        Size = SearchSize::Bits32;
        Compare = CompareType::Equal;

        _step = _previousSearch == nullptr ? 0 : _previousSearch->_step + 1;

        // Open file
        char    tid[17] = {0};
        Process::GetTitleID(tid);
        std::string path = "Search/";
        path += tid;
        path += "-Step" + std::to_string(_step) + ".bin";


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
    Search<T>::Search(T value, u32 start, u32 end, u32 alignment, SearchBase *prev) :
    SearchBase(start, end, alignment, prev)
    {
        _checkValue = value;
        _WriteHeaderToFile();
    }

    template <typename T>
    void      Search<T>::_FirstExactSearch(u32 &start, u32 end, u32 maxResult)
    {
        switch (Compare)
        {
            case CompareType::Equal:
            {
                for (; start < end; start += _alignment)
                {
                    T value = *((T *)(start));
                    if (value == _checkValue)
                    {
                        ResultCount++;
                        _results.push_back(SearchResult<T>(start, value));
                        if (_results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            case CompareType::NotEqual:
            {
                for (; start < end; start += _alignment)
                {
                    T value = *(reinterpret_cast<T *>(start));
                    if (value != _checkValue)
                    {
                        ResultCount++;
                        _results.push_back(SearchResult<T>(start, value));
                        if (_results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            case CompareType::GreaterThan:
            {
                for (; start < end; start += _alignment)
                {
                    T value = *(reinterpret_cast<T *>(start));
                    if (value > _checkValue)
                    {
                        ResultCount++;
                        _results.push_back(SearchResult<T>(start, value));
                        if (_results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            case CompareType::GreaterOrEqual:
            {
                for (; start < end; start += _alignment)
                {
                    T value = *(reinterpret_cast<T *>(start));
                    if (value >= _checkValue)
                    {
                        ResultCount++;
                        _results.push_back(SearchResult<T>(start, value));
                        if (_results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            case CompareType::LesserThan:
            {
                for (; start < end; start += _alignment)
                {
                    T value = *(reinterpret_cast<T *>(start));
                    if (value < _checkValue)
                    {
                        ResultCount++;
                        _results.push_back(SearchResult<T>(start, value));
                        if (_results.size() >= maxResult)
                            break;
                    }
                }
                break;
            }
            case CompareType::LesserOrEqual:
            {
                for (; start < end; start += _alignment)
                {
                    T value = *(reinterpret_cast<T *>(start));
                    if (value <= _checkValue)
                    {
                        ResultCount++;
                        _results.push_back(SearchResult<T>(start, value));
                        if (_results.size() >= maxResult)
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
    bool    Search<T>::_Compare(T older, T newer)
    {
        if (Type == SearchType::ExactValue)
        {
            switch (Compare)
            {
                case CompareType::Equal: return (_checkValue == newer);
                case CompareType::NotEqual: return (_checkValue != newer);
                case CompareType::GreaterThan: return (newer > _checkValue);
                case CompareType::GreaterOrEqual: return (newer >= _checkValue);
                case CompareType::LesserThan: return (newer < _checkValue);
                case CompareType::LesserOrEqual: return (newer <= _checkValue);
                case CompareType::DifferentBy: return (newer == (older + _checkValue) || newer == (older - _checkValue));
                case CompareType::DifferentByLess: return ((older - _checkValue) < newer && newer < (older + _checkValue));
                case CompareType::DifferentByMore: return (newer < (older - _checkValue) || newer > (older + _checkValue));
                default: return (false);
            } 
        }
        else
        {
            switch (Compare)
            {
                case CompareType::Equal: return (older == newer);
                case CompareType::NotEqual: return (older != newer);
                case CompareType::GreaterThan: return (newer > older);
                case CompareType::GreaterOrEqual: return (newer >= older);
                case CompareType::LesserThan: return (newer < older);
                case CompareType::LesserOrEqual: return (newer <= older);
                case CompareType::DifferentBy: return (newer == (older + _checkValue) || newer == (older - _checkValue));
                case CompareType::DifferentByLess: return ((older - _checkValue) < newer && newer < (older + _checkValue));
                case CompareType::DifferentByMore: return (newer < (older - _checkValue) || newer > (older + _checkValue));
                default: return (false);
            }
        }

    }

    template<typename T>
    bool    Search<T>::DoSearch(void)
    {
        _results.clear();

        // First Search ?
        if (_previousSearch == nullptr)
        {
            u32 start = _currentPosition == 0 ? _startRange : _currentPosition;
            u32 maxResult = 0x2000 / _GetResultStructSize();
            u32 end = start + std::min((u32)(_endRange - start), (u32)0x2000);

            if (Type == SearchType::ExactValue)
            {
                _FirstExactSearch(start, end, maxResult);

                if (_results.size())
                    ResultsToFile();

                // Update position
                _currentPosition = start;
            }                
            else // Unknown value
            {
                for (; start < end; start += _alignment)
                {
                    T value = *(reinterpret_cast<T *>(start));

                    ResultCount++;
                    _results.push_back(SearchResult<T>(start, value));
                    if (_results.size() >= maxResult)
                        break;
                }

                // Update position
                _currentPosition = start;
            }

            // Calculate progress
            //float chunk = (_endRange - _startRange);
            Progress = ((100.0f * (float)(_currentPosition - _startRange)) / (_endRange - _startRange));
        }
        // Second search
        else if (_previousSearch != nullptr)
        {
            
            std::vector<SearchResult<T>>    oldResults;

            u32 maxResult = 0x2000 / _GetResultStructSize();

            // First get the results from the file
            if (reinterpret_cast<Search<T> *>(_previousSearch)->_ReadResults(oldResults, _currentPosition, maxResult))
            {
                ResultIter iter = oldResults.begin();
                ResultIter end = oldResults.end();

                for (; iter != end; iter++)
                {
                    T val = *reinterpret_cast<T *>((*iter).address);
                    T oldVal = (*iter).value;
                    if (_Compare(oldVal, val))
                    {
                        ResultCount++;
                        _results.push_back(SearchResult<T>((*iter).address, val));
                    }
                }

                // Update position
                _currentPosition += oldResults.size();

                // Calculate progress
                float chunk = _previousSearch->ResultCount / 100;
                Progress = (float)(_currentPosition) / chunk;
            }
        }

        if (_currentPosition >= _endRange)
            return (true);

        return (false);
    }

    template <typename T>
    bool    Search<T>::_WriteHeaderToFile(void)
    {
        u64     offset = _file.Tell();
        int     ret = 0;

        _file.Rewind();

        ret |= WriteToFile(_file, Type);
        ret |= WriteToFile(_file, Compare);
        ret |= WriteToFile(_file, _startRange);
        ret |= WriteToFile(_file, _endRange);
        ret |= WriteToFile(_file, _alignment);
        ret |= WriteToFile(_file, ResultCount);
        ret |= WriteToFile(_file, Size);
        ret |= WriteToFile(_file, sizeof(T));
        ret |= WriteToFile(_file, _GetHeaderSize());
        u32 size = sizeof(u32) + sizeof(T);
        ret |= WriteToFile(_file, size);
        ret |= WriteToFile(_file, _checkValue);

        if (offset != 0)
            _file.Seek(offset, File::SeekPos::SET);

        if (ret != 0)
            return (false);

        return (true);
    }

    template <typename T>
    u32     Search<T>::_GetHeaderSize(void)
    {
        return 
        (  
            sizeof(Type)
            + sizeof(Compare)
            + sizeof(_startRange)
            + sizeof(_endRange)
            + sizeof(_alignment)
            + sizeof(ResultCount)
            + sizeof(Size)
            + sizeof(T)
            + sizeof(u32) // offset in file where results starts
            + sizeof(u32) // SearchResultSize
            + sizeof(T) // value compared with
        );
    }

    template <typename T>
    u32     Search<T>::_GetResultStructSize(void)
    {
        return 
        (
            sizeof(u32) // address
            + sizeof(T)
        );
    }

    template <typename T>
    bool    Search<T>::ResultsToFile(void)
    {
        // Write Header
        _WriteHeaderToFile();

        // Write all results
        _file.Write(_results.data(), _results.size() * _GetResultStructSize());
    }

    template <typename T>
    bool    Search<T>::_ReadResults(std::vector<SearchResult<T>> &out, u32 index, u32 count)
    {
        if (index > ResultCount)
            return (false);

        if (index + count > ResultCount)
            count = ResultCount - index;

        u64 offset = _GetHeaderSize() + _GetResultStructSize() * index;

        // Go to the file offset
        _file.Seek(offset, File::SeekPos::SET);

        // Reserve memory and create default object
        out.resize(count);

        //Read results
        if (_file.Read((void *)out.data(), count * _GetResultStructSize()) != 0)
            return (true);

        return (false);
    }

    template class Search<u8>;
    template class Search<u16>;
    template class Search<u32>;
    template class Search<u64>;
    template class Search<float>;
    template class Search<double>;
}
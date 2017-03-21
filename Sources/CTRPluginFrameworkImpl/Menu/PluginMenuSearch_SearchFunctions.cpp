#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearchStructs.hpp"
#include "CTRPluginFramework/System/Directory.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "ctrulib/allocator/linear.h"
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
        _fullRamSearch = false;
        _currentPosition = 0;
        _currentRegion = -1;
        _totalSize = 0;
        _achievedSize = 0;
        ResultCount = 0;

        // Init hit list variable
        _lastFetchedIndex = 0;
        _lastStartIndexInFile = 0;
        _lastEndIndexInFile = 0;

        // Default search parameters
        Type = SearchType::ExactValue;
        Size = SearchSize::Bits32;
        Compare = CompareType::Equal;

        Step = _previousSearch == nullptr ? 0 : _previousSearch->Step + 1;

        // Open file
        char    tid[17] = {0};
        Process::GetTitleID(tid);
        std::string path = "Search/";
        path += tid;
        path += "-Step" + std::to_string(Step) + ".bin";


        // Open current directory
        Directory dir;
        if (Directory::Open(dir, "Search", true) == 0)
        {
            // Open file
            File::Open(_file, path, File::READ | File::WRITE | File::CREATE | File::TRUNCATE);
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
        //_WriteHeaderToFile();

        _resultsArray = (SearchResult<T>*)linearAlloc(0x40000); //256 KB
        _maxResult = (0x40000 / sizeof(SearchResult<T>)) - 1;
        _resultsEnd = _resultsArray + _maxResult;
        _resultsP = _resultsArray;
    }

    template <typename T>
    Search<T>::Search(T value, std::vector<Region> &list, u32 alignment, SearchBase *prev) :
    SearchBase(0, 0x50000000, alignment, prev)
    {
        _checkValue = value;
        //_WriteHeaderToFile();

        _resultsArray = (SearchResult<T>*)linearAlloc(0x40000); //256 KB
        _maxResult = (0x40000 / sizeof(SearchResult<T>)) - 1;
        _resultsEnd = _resultsArray + _maxResult;
        _resultsP = _resultsArray;

        _regionsList = list;
        _fullRamSearch = true;
        // Get Total Size
        for (int i = 0; i < list.size(); i++)
        {
            Region &region = list[i];
            int size = region.endAddress - region.startAddress;

            _totalSize += size;
        }
    }

    /*
    **
    *****************************/

    template <typename T>
    void    Search<T>::_UpdateCompare(void)
    {
        if (Type == SearchType::ExactValue)
        {
            switch (Compare)
            {
                case CompareType::Equal: _compare = &Search<T>::_CompareE; break;
                case CompareType::NotEqual: _compare = &Search<T>::_CompareNE; break;
                case CompareType::GreaterThan: _compare = &Search<T>::_CompareGT; break;
                case CompareType::GreaterOrEqual: _compare = &Search<T>::_CompareGE; break;
                case CompareType::LesserThan: _compare = &Search<T>::_CompareLT; break;
                case CompareType::LesserOrEqual: _compare = &Search<T>::_CompareLE; break;
                case CompareType::DifferentBy: _compare = &Search<T>::_CompareDB; break;
                case CompareType::DifferentByLess: _compare = &Search<T>::_CompareDBL; break;
                case CompareType::DifferentByMore: _compare = &Search<T>::_CompareDBM; break;
                default: _compare = &Search<T>::_CompareE; break;
            } 
        }
        else
        {
            switch (Compare)
            {
                case CompareType::Equal: _compare = &Search<T>::_CompareUnknownE; break;
                case CompareType::NotEqual: _compare = &Search<T>::_CompareUnknownNE; break;
                case CompareType::GreaterThan: _compare = &Search<T>::_CompareUnknownGT; break;
                case CompareType::GreaterOrEqual: _compare = &Search<T>::_CompareUnknownGE; break;
                case CompareType::LesserThan: _compare = &Search<T>::_CompareUnknownLT; break;
                case CompareType::LesserOrEqual: _compare = &Search<T>::_CompareUnknownLE; break;
                case CompareType::DifferentBy: _compare = &Search<T>::_CompareDB; break;
                case CompareType::DifferentByLess: _compare = &Search<T>::_CompareDBL; break;
                case CompareType::DifferentByMore: _compare = &Search<T>::_CompareDBM; break;
                default: _compare = &Search<T>::_CompareUnknownE; break;
            }
        }
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
                        _resultsP->address = start;
                        _resultsP->value = value;
                        _resultsP++;
                        if (_resultsP >= _resultsEnd)
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
                        _resultsP->address = start;
                        _resultsP->value = value;
                        _resultsP++;
                        if (_resultsP >= _resultsEnd)
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
                        _resultsP->address = start;
                        _resultsP->value = value;
                        _resultsP++;
                        if (_resultsP >= _resultsEnd)
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
                        _resultsP->address = start;
                        _resultsP->value = value;
                        _resultsP++;
                        if (_resultsP >= _resultsEnd)
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
                        _resultsP->address = start;
                        _resultsP->value = value;
                        _resultsP++;
                        if (_resultsP >= _resultsEnd)
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
                        _resultsP->address = start;
                        _resultsP->value = value;
                        _resultsP++;
                        if (_resultsP >= _resultsEnd)
                            break;
                    }
                }
                break;
            }
            default:
                break;
        } // End switch
        //if (_resultsP >= _resultsEnd)
          //  start += _alignment;
    }

    template<typename T>
    bool    Search<T>::_SearchInRange(void)
    {
        // First Search ?
        if (_previousSearch == nullptr)
        {
            u32 start = _currentPosition == 0 ? _startRange : _currentPosition;
            
            u32 end = start + std::min((u32)(_endRange - start), (u32)0x40000);

            if (Type == SearchType::ExactValue)
            {
                _FirstExactSearch(start, end, _maxResult);

                // Update position
                _currentPosition = start;
            }                
            else // Unknown value
            {             
                for (; start < end; start += _alignment)
                {
                    T value = *(reinterpret_cast<T *>(start));

                    ResultCount++;
                    _resultsP->address = start;
                    _resultsP->value = value;
                    _resultsP++;
                    //_results.push_back(SearchResult<T>(start, value));   _results.size() >= _maxResult
                    if (_resultsP >= _resultsEnd)
                        break;
                }

                // Update position
                _currentPosition = start;
            }

            // Finish
            if (_currentPosition >= _endRange)
            {
                return (true);
            }
        }
        // Subsidiary search
        else
        {            
            std::vector<SearchResult<T>>    oldResults;

            // First get the results from the file
            if (reinterpret_cast<Search<T> *>(_previousSearch)->_ReadResults(oldResults, _currentPosition, _maxResult))
            {
                ResultIter iter = oldResults.begin();
                ResultIter end = oldResults.end();
                u32 count = 0;

                for (; iter != end; iter++, count++)
                {
                    u32 addr = (*iter).address;
                    T   newval = *reinterpret_cast<T *>(addr);
                    T   oldVal = (*iter).value;

                    if ((this->*_compare)(oldVal, newval))
                    {
                        ResultCount++;
                        _resultsP->address = addr;
                        _resultsP->value = newval;
                        _resultsP->oldValue = oldVal;
                        _resultsP++;

                        if (_resultsP >= _resultsEnd)
                        {
                            count++;
                            break;
                        }                        
                        //_results.push_back(SearchResult<T>((*iter).address, newval));
                    }
                }
                // Update position
                _currentPosition += count;
            }

            // Finish
            if (_currentPosition >= _previousSearch->ResultCount)
            {
                return (true);
            }
        }

        return (false);
    }

    template<typename T>
    bool    Search<T>::DoSearch(void)
    {
        // Just started search ?
        if (_currentPosition == 0)
        {
            _UpdateCompare();
            _WriteHeaderToFile();
            _clock.Restart();
        }

        bool    finished = false;
        // Range search
        if (!_fullRamSearch)
        {
            finished = _SearchInRange();

            // First Search
            if (_previousSearch == nullptr)
            {
                // Calculate progress
                Progress = ((100.0f * (float)(_currentPosition - _startRange)) / (_endRange - _startRange));
            }
            else
            {
                // Calculate progress
                Progress = (100.f * (float)(_currentPosition)) / _previousSearch->ResultCount;
            }

            // Finish
            if (finished)
            {
                ResultsToFile();
                _WriteHeaderToFile();
                SearchTime = _clock.Restart();

                // Free buffer
                linearFree(_resultsArray);
            }
            else
            {
                if (_resultsP >= _resultsEnd)
                    ResultsToFile();
            }
            return (finished);
        }
        // All regions search
        else
        {
            // Start search
            if (_previousSearch == nullptr)
            {
                if (_currentRegion == -1)
                {                
                    _currentRegion = 0;
                    _startRange = _regionsList[0].startAddress;
                    _endRange = _regionsList[0].endAddress;
                    _currentPosition = _startRange;
                    svcFlushProcessDataCache(Process::GetHandle(), (void *)_startRange, _endRange - _startRange);
                }
            }


            // Save current position
            u32 position = _currentPosition;

            // Do search
            finished = _SearchInRange();

            // Compute size done
            u32 size = _currentPosition - position;
            _achievedSize += size;

            // Calculate Progress
            if (_previousSearch == nullptr)
                Progress = ((100.0f * (float)(_achievedSize)) / (_totalSize));
            else
                Progress = (100.f * (float)(_currentPosition)) / _previousSearch->ResultCount;

            // If finished, set next region parameters
            if (finished)
            {
                // If we didn't finished the last region
                if (_previousSearch == nullptr && _currentRegion < _regionsList.size() - 1)
                {
                    _currentRegion++;
                    _startRange = _regionsList[_currentRegion].startAddress;
                    _endRange = _regionsList[_currentRegion].endAddress;
                    _currentPosition = _startRange;
                    svcFlushProcessDataCache(Process::GetHandle(), (void *)_startRange, _endRange - _startRange);
                }
                else
                {
                    ResultsToFile();
                    _WriteHeaderToFile();
                    SearchTime = _clock.Restart();

                    // Free buffer
                    linearFree(_resultsArray);

                    return (true);
                }

            }
            
            if (_resultsP >= _resultsEnd)
                ResultsToFile();

            return (false);
        }
    }

    template <typename T>
    struct Header
    {
        u8      type;
        u8      compare;
        u32     startRange;
        u32     endRange;
        u8      alignment;
        u32     resultcount;
        u8      size;
        u8      valuesize;
        u32     offset;
        u32     resultSize;
        T       value;
    };

    template <typename T>
    bool    Search<T>::_WriteHeaderToFile(void)
    {
        u64     offset = _file.Tell();
        int     ret = 0;

        _file.Rewind();

        Header<T>   header;

        header.type = (u8)Type;
        header.compare = (u8)Compare;
        header.startRange = _startRange;
        header.endRange = _endRange;
        header.alignment = _alignment;
        header.resultcount = ResultCount;
        header.size = (u8)Size;
        header.valuesize = sizeof(T);
        header.offset = sizeof(Header<T>);
        header.resultSize = sizeof(SearchResult<T>);
        header.value = _checkValue;

        /*ret |= WriteToFile(_file, Type);                // u8
        ret |= WriteToFile(_file, Compare);             // u8
        ret |= WriteToFile(_file, _startRange);         // u32
        ret |= WriteToFile(_file, _endRange);           // u32
        ret |= WriteToFile(_file, _alignment);          // u8
        ret |= WriteToFile(_file, ResultCount);         // u32
        ret |= WriteToFile(_file, Size);                // u8
        ret |= WriteToFile(_file, (u8)sizeof(T));       // u8
        ret |= WriteToFile(_file, _GetHeaderSize());    // u32
        u32 size = _GetResultStructSize();// sizeof(u32) + sizeof(T);        
        ret |= WriteToFile(_file, size);                // u32
        ret |= WriteToFile(_file, _checkValue);         // T*/

        ret = WriteToFile(_file, header);

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
           /* sizeof(Type) // u8
            + sizeof(Compare) // u8
            + sizeof(_startRange) // u32
            + sizeof(_endRange) //u32
            + sizeof(_alignment) // u8
            + sizeof(ResultCount) //u32
            + sizeof(Size) // u8
            + sizeof(u8) // u8 //sizeof(T)
            + sizeof(u32) // offset in file where results starts
            + sizeof(u32) // SearchResultSize
            + sizeof(T) // value compared with*/
            sizeof(Header<T>)
        );
    }

    template <typename T>
    u32     Search<T>::_GetResultStructSize(void)
    {
        /*u32 size = sizeof(u32) + sizeof(T);

        size += size % 4;*/
        return (sizeof(SearchResult<T>));
    }

    template <typename T>
    bool    Search<T>::ResultsToFile(void)
    {
        // Write Header
        //_WriteHeaderToFile();

        // Write all results
        //_file.Write(_results.data(), _results.size() * _GetResultStructSize());
        _file.Write(_resultsArray, (_resultsP - _resultsArray) * _GetResultStructSize());
        _resultsP = _resultsArray;
        // Clear results vector
        //_results.clear();
    }

    template <typename T>
    bool    Search<T>::_ReadResults(std::vector<SearchResult<T>> &out, u32 &index, u32 count)
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

        // Read results
        if (_file.Read((void *)out.data(), count * _GetResultStructSize()) == 0)
        {
            index += count;
            return (true);
        }

        // Clean results
        out.clear();

        return (false);
    }

    template <typename T>
    void    Search<T>::Cancel(void)
    {
        ResultsToFile();
        SearchTime = _clock.Restart();

        // Free buffer
        linearFree(_resultsArray);
    }

    template <typename T>
    bool    Search<T>::FetchResults(stringvector &address, stringvector &newval, stringvector &oldvalue, u32 index, int count)
    {
        std::vector<SearchResult<T>>    newResults;
        char   buffer[20] = {0};

        // Fetch new results
        if (!_ReadResults(newResults, index, count))
            return (false);

        // First search
        if (_previousSearch == nullptr)
        {
            for (int i = 0; i < newResults.size(); i++)
            {
                // Address
                sprintf(buffer, "%08X", newResults[i].address);
                address.push_back(buffer);

                // Value
                switch (Size)
                {
                    case SearchSize::Bits8: sprintf(buffer, "%02X", newResults[i].value); break;
                    case SearchSize::Bits16: sprintf(buffer, "%04X", newResults[i].value); break;
                    case SearchSize::Bits32: sprintf(buffer, "%08X", newResults[i].value); break;
                    case SearchSize::Bits64: sprintf(buffer, "%016llX", newResults[i].value); break;
                    case SearchSize::FloatingPoint: sprintf(buffer, "%8.7f", newResults[i].value); break;
                    case SearchSize::Double: sprintf(buffer, "%8.7g", newResults[i].value); break;
                }
                newval.push_back(buffer);
            } 
        }
        // subsidiary
        else
        {
            for (int i = 0; i < newResults.size(); i++)
            {
                // Address
                sprintf(buffer, "%08X", newResults[i].address);
                address.push_back(buffer);

                // Value
                switch (Size)
                {
                    case SearchSize::Bits8: sprintf(buffer, "%02X", newResults[i].value); break;
                    case SearchSize::Bits16: sprintf(buffer, "%04X", newResults[i].value); break;
                    case SearchSize::Bits32: sprintf(buffer, "%08X", newResults[i].value); break;
                    case SearchSize::Bits64: sprintf(buffer, "%016llX", newResults[i].value); break;
                    case SearchSize::FloatingPoint: sprintf(buffer, "%8.7f", newResults[i].value); break;
                    case SearchSize::Double: sprintf(buffer, "%8.7g", newResults[i].value); break;
                }
                newval.push_back(buffer);

                // Old Value
                switch (Size)
                {
                    case SearchSize::Bits8: sprintf(buffer, "%02X", newResults[i].oldValue); break;
                    case SearchSize::Bits16: sprintf(buffer, "%04X", newResults[i].oldValue); break;
                    case SearchSize::Bits32: sprintf(buffer, "%08X", newResults[i].oldValue); break;
                    case SearchSize::Bits64: sprintf(buffer, "%016llX", newResults[i].oldValue); break;
                    case SearchSize::FloatingPoint: sprintf(buffer, "%8.7f", newResults[i].oldValue); break;
                    case SearchSize::Double: sprintf(buffer, "%8.7g", newResults[i].oldValue); break;
                }
                oldvalue.push_back(buffer);
            } 
        }
        // Subsidiary search (need to loads from file)
      /*  else if (_previousSearch->Type == SearchType::ExactValue && _previousSearch->Compare == CompareType::Equal)
        {
            T   oldValue = reinterpret_cast<Search<T> *>(_previousSearch)->_checkValue;
            char                            oldBuffer[20] = {0};

            // Format old value
            switch (Size)
            {
                case SearchSize::Bits8: sprintf(oldBuffer, "%02X", oldValue); break;
                case SearchSize::Bits16: sprintf(oldBuffer, "%04X", oldValue); break;
                case SearchSize::Bits32: sprintf(oldBuffer, "%08X", oldValue); break;
                case SearchSize::Bits64: sprintf(oldBuffer, "%016llX", oldValue); break;
                case SearchSize::FloatingPoint: sprintf(oldBuffer, "%8.7f", oldValue); break;
                case SearchSize::Double: sprintf(oldBuffer, "%8.7g", oldValue); break;
            }

            for (int i = 0; i < newResults.size(); i++)
            {
                // Address
                sprintf(buffer, "%08X", newResults[i].address);
                address.push_back(buffer);

                // Value
                switch (Size)
                {
                    case SearchSize::Bits8: sprintf(buffer, "%02X", newResults[i].value); break;
                    case SearchSize::Bits16: sprintf(buffer, "%04X", newResults[i].value); break;
                    case SearchSize::Bits32: sprintf(buffer, "%08X", newResults[i].value); break;
                    case SearchSize::Bits64: sprintf(buffer, "%016llX", newResults[i].value); break;
                    case SearchSize::FloatingPoint: sprintf(buffer, "%8.7f", newResults[i].value); break;
                    case SearchSize::Double: sprintf(buffer, "%8.7g", newResults[i].value); break;
                }
                newval.push_back(buffer);

                oldvalue.push_back(oldBuffer);
            } 
        }
        else
        {        
            std::vector<SearchResult<T>>    oldResults;
            u32                             oldIndex;
            bool                            reverse = false;

            // Try to narrow if it's not the first search
            if (_lastFetchedIndex == index) oldIndex = _lastStartIndexInFile;
            else if (_lastFetchedIndex < index) oldIndex = _lastEndIndexInFile;
            else if (_lastFetchedIndex > index) { oldIndex = _lastStartIndexInFile; reverse = true; }

            _lastFetchedIndex = index;

            // Load some of the old results
            reinterpret_cast<Search<T> *>(_previousSearch)->_ReadResults(oldResults, oldIndex, 0x1000, reverse);
            int y = 0;

            for (int i = 0; i < newResults.size(); i++)
            {
                // Address
                sprintf(buffer, "%08X", newResults[i].address);
                address.push_back(buffer);

                // new Value
                switch (Size)
                {
                    case SearchSize::Bits8: sprintf(buffer, "%02X", newResults[i].value); break;
                    case SearchSize::Bits16: sprintf(buffer, "%04X", newResults[i].value); break;
                    case SearchSize::Bits32: sprintf(buffer, "%08X", newResults[i].value); break;
                    case SearchSize::Bits64: sprintf(buffer, "%016llX", newResults[i].value); break;
                    case SearchSize::FloatingPoint: sprintf(buffer, "%8.7f", newResults[i].value); break;
                    case SearchSize::Double: sprintf(buffer, "%8.7g", newResults[i].value); break;
                }
                newval.push_back(buffer);

                u32 tofind = newResults[i].address;

                // Find same address in old results
                
                while (1)
                {
                    for (; y < oldResults.size(); y++)
                        if (oldResults[y].address == tofind)
                            break;

                    if (oldResults[y].address == tofind)
                        break;
                    if (!reinterpret_cast<Search<T> *>(_previousSearch)->_ReadResults(oldResults, oldIndex, 0x1000, reverse))
                    {
                        y = -1;
                        break;
                    }
                    else
                        y = 0;
                }

                if (y == -1)
                    continue;

                // old Value
                switch (Size)
                {
                    case SearchSize::Bits8: sprintf(buffer, "%02X", oldResults[y].value); break;
                    case SearchSize::Bits16: sprintf(buffer, "%04X", oldResults[y].value); break;
                    case SearchSize::Bits32: sprintf(buffer, "%08X", oldResults[y].value); break;
                    case SearchSize::Bits64: sprintf(buffer, "%016llX", oldResults[y].value); break;
                    case SearchSize::FloatingPoint: sprintf(buffer, "%8.7f", oldResults[y].value); break;
                    case SearchSize::Double: sprintf(buffer, "%8.7g", oldResults[y].value); break;
                }
                oldvalue.push_back(buffer);
                if (i == 0)
                {
                    _lastStartIndexInFile = oldIndex + y;
                    if (reverse)
                        _lastStartIndexInFile += oldResults.size();
                    else
                        _lastStartIndexInFile -= oldResults.size();
                }
                if (i == newResults.size() - 1)
                {
                    _lastEndIndexInFile = oldIndex + y;
                    if (reverse)
                        _lastEndIndexInFile += oldResults.size();
                    else
                        _lastEndIndexInFile -= oldResults.size();
                }
            } 
        }*/
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareE(T older, T newer)
    {
        return (_checkValue == newer);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareNE(T older, T newer)
    {
        return (_checkValue != newer);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareGT(T older, T newer)
    {
        return (newer > _checkValue);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareGE(T older, T newer)
    {
        return (newer >= _checkValue);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareLT(T older, T newer)
    {
        return (newer < _checkValue);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareLE(T older, T newer)
    {
        return (newer <= _checkValue);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareDB(T older, T newer)
    {
        return (newer == (older + _checkValue) || newer == (older - _checkValue));
    }

        // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareDBL(T older, T newer)
    {
        return ((older - _checkValue) < newer && newer < (older + _checkValue));
    }

        // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareDBM(T older, T newer)
    {
        return (newer < (older - _checkValue) || newer > (older + _checkValue));
    }

    /*
    ** Unknown
    *************/
    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareUnknownE(T older, T newer)
    {
        return (older == newer);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareUnknownNE(T older, T newer)
    {
        return (older != newer);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareUnknownGT(T older, T newer)
    {
        return (newer > older);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareUnknownGE(T older, T newer)
    {
        return (newer >= older);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareUnknownLT(T older, T newer)
    {
        return (newer < older);
    }

    // Return if it matches the condition
    template<typename T>
    bool    Search<T>::_CompareUnknownLE(T older, T newer)
    {
        return (newer <= older);
    }

    template class Search<u8>;
    template class Search<u16>;
    template class Search<u32>;
    template class Search<u64>;
    template class Search<float>;
    template class Search<double>;
}
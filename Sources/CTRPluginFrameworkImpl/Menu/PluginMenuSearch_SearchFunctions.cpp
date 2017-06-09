#include "CTRPluginFrameworkImpl/Menu/PluginMenuSearchStructs.hpp"
#include "CTRPluginFramework/System/Directory.hpp"
#include "CTRPluginFramework/System/Process.hpp"
#include "ctrulib/allocator/linear.h"

#include <cmath>
#include <string>
#include <cstring>

#include "CTRPluginFrameworkImpl/Preferences.hpp"
#include "CTRPluginFramework/Menu/MessageBox.hpp"

namespace CTRPluginFramework
{
    static std::vector<SRegion> g_regionList;

    /*
    ** SearchBase
    *******************/

    SearchBase::SearchBase(u32 start, u32 end, u32 alignment, SearchBase *previous) :
    _startRange(start), _endRange(end), _alignment(alignment), _previousSearch(previous), Progress(0.0f), _regionsList(g_regionList)
    {
        // Init search variables
        _fullRamSearch = false;
        _currentPosition = 0;
        _currentRegion = -1;
        _totalSize = 0;
        _achievedSize = 0;
        ResultCount = 0;
        PoolError = false;

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
        std::string path = "Search/";
        Process::GetTitleID(path);
        
        path += "-Step" + std::to_string(Step) + ".bin";


        // Open current directory
        Directory dir;
        if (Directory::Open(dir, "Search", true) == 0)
        {
            // Open file
            int res = File::Open(_file, path, File::READ | File::WRITE | File::CREATE | File::TRUNCATE);

            if (res != 0)
            {
                char buff[100];

                sprintf(buff, "Couldn't open %s\n%08X", path.c_str(), res);
                (MessageBox(buff))();
            }
        }
    }

    /*
    ** Search : SearchBase
    ************/

    void    *AllocatePool(u32 &poolSize)
    {
        u32     linearFreeSize = linearSpaceFree();
        u32     size = 0x80000;
        void    *pool = nullptr;

        if (linearFreeSize >= size)
            pool = linearAlloc(size);

        if (pool == nullptr)
        {
            // Try with the base heap
            pool = new u8[size];
            
            if (pool == nullptr)
            {
                size = 0x40000;

                while (pool == nullptr && size >= 0x10000)
                {
                    if ((pool = linearAlloc(size)) != nullptr)
                        break;
                    if ((pool = new u8[size]) != nullptr)
                        break;
                    size -= 0x10000;
                }
            }
        }

        if (pool == nullptr)
            poolSize = 0;
        else
            poolSize = size;

        return (pool);
    }

    template <typename T>
    Search<T>::Search(T value, u32 start, u32 end, u32 alignment, SearchBase *prev) :
    SearchBase(start, end, alignment, prev)
    {
        _checkValue = value;
        //_WriteHeaderToFile();

        if (prev == nullptr)
        {
            _regionsList.clear();
            _regionsList.push_back({ start, end, _GetHeaderSize() });
        }
        
        u32 size;// = Preferences::EcoMemoryMode ? 0x40000 : 0x80000;

        _resultsArray = AllocatePool(size);

        if (size == 0)
        {
            PoolError = true;
            return;
        }
            
        _resultsP = _resultsArray;
        _resultsEnd = (void *)((u32)_resultsArray + size);
        _maxResult = size;

        _flags = Subsidiary;
    }

    template <typename T>
    Search<T>::Search(T value, std::vector<Region> &list, u32 alignment, SearchBase *prev) :
    SearchBase(0, 0x50000000, alignment, prev)
    {
        _checkValue = value;
        //_WriteHeaderToFile();

        u32 size;// = Preferences::EcoMemoryMode ? 0x40000 : 0x80000;

        _resultsArray = AllocatePool(size);

        if (size == 0)
        {
            PoolError = true;
            return;
        }

        _resultsP = _resultsArray;
        _resultsEnd = (void *)((u32)_resultsArray + size);
        _maxResult = size;

        _fullRamSearch = true;
        _flags = Subsidiary;

        if (prev == nullptr)
        {
            _regionsList.clear();
        }
        
        // Get Total Size
        for (int i = 0; i < list.size(); i++)
        {
            Region &region = list[i];

            if (prev == nullptr)
                _regionsList.push_back({ region.startAddress, region.endAddress, 0 });

            size = region.endAddress - region.startAddress;
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
    void    Search<T>::_FirstExactSearch(u32 &start, const u32 end)
    {
        SearchResultFirst<T>    *result = reinterpret_cast<SearchResultFirst<T>*>(_resultsP);
        //u32 count = 0;
        switch (Compare)
        {
            case CompareType::Equal:
            {
                for (; start < end; start += _alignment)
                {
                    T value = *(reinterpret_cast<T *>(start));
                    if (value == _checkValue)
                    {
                        ResultCount++;
                        result->address = start;
                        result->value = value;
                        ++result;
                       /* count++;
                        if (count >= _maxResult)
                            break;
                        */
                        if ((u32)result >= (u32)_resultsEnd)
                        {
                            start += _alignment;
                            break;
                        }
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
                        result->address = start;
                        result->value = value;
                        ++result;
                        if ((u32)result >= (u32)_resultsEnd)
                        {
                            start += _alignment;
                            break;
                        }
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
                        result->address = start;
                        result->value = value;
                        ++result;
                        if ((u32)result >= (u32)_resultsEnd)
                        {
                            start += _alignment;
                            break;
                        }
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
                        result->address = start;
                        result->value = value;
                        ++result;
                        if ((u32)result >= (u32)_resultsEnd)
                        {
                            start += _alignment;
                            break;
                        }
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
                        result->address = start;
                        result->value = value;
                        ++result;
                        if ((u32)result >= (u32)_resultsEnd)
                        {
                            start += _alignment;
                            break;
                        }
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
                        result->address = start;
                        result->value = value;
                        ++result;
                        if ((u32)result >= (u32)_resultsEnd)
                        {
                            start += _alignment;
                            break;
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
        _resultsP = result;
    }

    template <>
    void    Search<float>::_FirstExactSearch(u32 &start, const u32 end)
    {
        SearchResultFirst<float>    *result = reinterpret_cast<SearchResultFirst<float>*>(_resultsP);
        float epsilon = 1e-6f;
        
        switch (Compare)
        {
        case CompareType::Equal:
        {
            for (; start < end; start += _alignment)
            {
                float value = *(reinterpret_cast<float *>(start));

                if (std::isnan(value) || std::isinf(value))
                    continue;

                if (std::fabs(value - _checkValue) <= epsilon)
                {
                    ResultCount++;
                    result->address = start;
                    result->value = value;
                    result++;
                    if ((u32)result >= (u32)_resultsEnd)
                    {
                        start += _alignment;
                        break;
                    }
                }
            }
            break;
        }
        case CompareType::NotEqual:
        {
            for (; start < end; start += _alignment)
            {
                float value = *(reinterpret_cast<float *>(start));

                if (std::isnan(value) || std::isinf(value))
                    continue;

                if (std::fabs(value - _checkValue) > epsilon)
                {
                    ResultCount++;
                    result->address = start;
                    result->value = value;
                    result++;
                    if ((u32)result >= (u32)_resultsEnd)
                    {
                        start += _alignment;
                        break;
                    }
                }
            }
            break;
        }
        case CompareType::GreaterThan:
        {
            for (; start < end; start += _alignment)
            {
                float value = *(reinterpret_cast<float *>(start));

                if (std::isnan(value) || std::isinf(value))
                    continue;

                if (value > _checkValue)
                {
                    ResultCount++;
                    result->address = start;
                    result->value = value;
                    ++result;
                    if ((u32)result >= (u32)_resultsEnd)
                    {
                        start += _alignment;
                        break;
                    }
                }
            }
            break;
        }
        case CompareType::GreaterOrEqual:
        {
            for (; start < end; start += _alignment)
            {
                float value = *(reinterpret_cast<float *>(start));

                if (std::isnan(value) || std::isinf(value))
                    continue;

                if (value >= _checkValue)
                {
                    ResultCount++;
                    result->address = start;
                    result->value = value;
                    ++result;
                    if ((u32)result >= (u32)_resultsEnd)
                    {
                        start += _alignment;
                        break;
                    }
                }
            }
            break;
        }
        case CompareType::LesserThan:
        {
            for (; start < end; start += _alignment)
            {
                float value = *(reinterpret_cast<float *>(start));

                if (std::isnan(value) || std::isinf(value))
                    continue;

                if (value < _checkValue)
                {
                    ResultCount++;
                    result->address = start;
                    result->value = value;
                    ++result;
                    if ((u32)result >= (u32)_resultsEnd)
                    {
                        start += _alignment;
                        break;
                    }
                }
            }
            break;
        }
        case CompareType::LesserOrEqual:
        {
            for (; start < end; start += _alignment)
            {
                float value = *(reinterpret_cast<float *>(start));

                if (std::isnan(value) || std::isinf(value))
                    continue;

                if (value <= _checkValue)
                {
                    ResultCount++;
                    result->address = start;
                    result->value = value;
                    ++result;
                    if ((u32)result >= (u32)_resultsEnd)
                    {
                        start += _alignment;
                        break;
                    }
                }
            }
            break;
        }
        default:
            break;
        }
        _resultsP = result;
    }

    template <typename T>
    void    Search<T>::_FirstUnknownSearch(u32 &start, const u32 end)
    {
        SearchResultUnknown<T> *result = reinterpret_cast<SearchResultUnknown<T>*>(_resultsP);

        for (; start < end; start += _alignment)
        {
            T value = *(reinterpret_cast<T *>(start));

            ResultCount++;
            result->value = value;
            ++result;
            if (result >= _resultsEnd)
            {
                start += _alignment;
                break;
            }

        }
        _resultsP = result;
    }

    template <>
    void    Search<float>::_FirstUnknownSearch(u32 &start, const u32 end)
    {
        SearchResultUnknown<float> *result = reinterpret_cast<SearchResultUnknown<float>*>(_resultsP);

        for (; start < end; start += _alignment)
        {
            float value = *(reinterpret_cast<float *>(start));

            if (std::isnan(value) || std::isinf(value))
                continue;

            ResultCount++;
            result->value = value;
            ++result;
            if (result >= _resultsEnd)
            {
                start += _alignment;
                break;
            }

        }
        _resultsP = result;
    }
    /*
    template <>
    void    Search<u8>::_FirstUnknownSearch(u32 &start, const u32 end)
    {
        SearchResultUnknown<u8> *result = reinterpret_cast<SearchResultUnknown<u8>*>(_resultsP);

        for (; start < end; start += 4)
        {
            u32 value = *(reinterpret_cast<u32 *>(start));

            ResultCount++;
            result->value = value;
            ++result;
            if (result >= _resultsEnd)
            {
                start += 4;
                break;
            }

        }
        _resultsP = result;
    }

    template <>
    void    Search<u16>::_FirstUnknownSearch(u32 &start, const u32 end)
    {
        SearchResultUnknown<u16> *result = reinterpret_cast<SearchResultUnknown<u16>*>(_resultsP);

        for (; start < end; start += 4)
        {
            u32 value = *(reinterpret_cast<u32 *>(start));

            ResultCount++;
            result->value = value;
            ++result;
            if (result >= _resultsEnd)
            {
                start += 4;
                break;
            }

        }
        _resultsP = result;
    } */

    template <typename T>
    bool    Search<T>::_SecondExactSearch(void)
    {
        std::vector<SearchResultFirst<T>>   oldResultsFirst;

        u32 index = _currentPosition;
        // First get the results from the file
        if (reinterpret_cast<Search<T> *>(_previousSearch)->_ReadResults(oldResultsFirst, index, _maxResult))
        {
            SearchResult<T>     *result = reinterpret_cast<SearchResult<T>*>(_resultsP);
            ResultExactIter iter = oldResultsFirst.begin();
            ResultExactIter end = oldResultsFirst.end();
            u32 count = 1;

            for (; iter != end; ++iter, ++count)
            {
                u32 addr = iter->address;
                T   newval = *reinterpret_cast<T *>(addr);
                T   oldVal = iter->value;

                if ((this->*_compare)(oldVal, newval))
                {
                    ++ResultCount;
                    result->address = addr;
                    result->value = newval;
                    result->oldValue = oldVal;
                    ++result;

                    if (result >= _resultsEnd)
                        break;
                }
            }
            _resultsP = result;
            // Update position
            _currentPosition += count;

            if (_currentPosition >= _previousSearch->ResultCount)
            {
                return (true);
            }
        }

        return (false);
    }

#define LOG(str, ...) {char buf[0x100]; sprintf(buf, str, ##__VA_ARGS__); (MessageBox(buf))();}
    template <typename T>
    bool    Search<T>::_SecondUnknownSearch(void)
    {
        std::vector<SearchResultUnknown<T>> oldResultsUnkn;

        auto getAddress = [this](u32 index, u32 &addr, u32 &end, u32 &nextAddr, u32 &nextEndAddr)
        {
            Search<T> *prev = reinterpret_cast<Search<T> *>(_previousSearch);

            u64 offset = prev->_GetHeaderSize() + prev->_GetResultStructSize() * index;
            u32 size = prev->_GetResultStructSize();

            for (SRegion &region : prev->_regionsList)
            {
                if (offset >= region.fileOffset)
                {
                    addr = region.startAddress;
                    addr += (offset - region.fileOffset);
                    end = region.endAddress;
                    nextEndAddr = end;
                }
                else
                {
                    nextAddr = region.startAddress;
                    nextEndAddr = region.endAddress;
                    break;
                }
            }
        };

        u32 address;
        u32 endAddr;
        u32 nextAddr;
        u32 nextEndAddr;
        u32 index = _currentPosition;// == 0 ? _currentPosition : _currentPosition - 1;

        getAddress(index, address, endAddr, nextAddr, nextEndAddr);
       // LOG("index: %08X\nAddress: %08X", index, address);
        
        // First get the results from the file
        if (reinterpret_cast<Search<T> *>(_previousSearch)->_ReadResults(oldResultsUnkn, index, _maxResult))
        {
            SearchResult<T>     *result = reinterpret_cast<SearchResult<T>*>(_resultsP);
            ResultUnknownIter iter = oldResultsUnkn.begin();
            ResultUnknownIter end = oldResultsUnkn.end();
            u32 count = 1;

            for (; iter != end; ++iter, ++count)
            {
                T   newval = *reinterpret_cast<T *>(address);
                T   oldVal = iter->value;

                if ((this->*_compare)(oldVal, newval))
                {
                    ++ResultCount;
                    result->address = address;
                    result->value = newval;
                    result->oldValue = oldVal;
                    ++result;

                    if (result >= _resultsEnd)
                        break;
                }

                address += _alignment;
                if (address >= endAddr)
                {
                    if (endAddr == nextEndAddr)
                        break;

                    address = nextAddr;
                    endAddr = nextEndAddr;
                }
            }

            _resultsP = result;

            // Update position
            _currentPosition += count;

            if (_currentPosition >= _previousSearch->ResultCount)
                return (true);
        }

        return (false);
    }

    template <>
    bool    Search<float>::_SecondUnknownSearch(void)
    {
        std::vector<SearchResultUnknown<float>> oldResultsUnkn;

        auto getAddress = [this](u32 index, u32 &addr, u32 &end, u32 &nextAddr, u32 &nextEndAddr)
        {
            Search<float> *prev = reinterpret_cast<Search<float> *>(_previousSearch);

            u64 offset = prev->_GetHeaderSize() + prev->_GetResultStructSize() * index;
            u32 size = prev->_GetResultStructSize();

            for (SRegion &region : prev->_regionsList)
            {
                if (offset >= region.fileOffset)
                {
                    addr = region.startAddress;
                    addr += (offset - region.fileOffset);
                    end = region.endAddress;
                    nextEndAddr = end;
                }
                else
                {
                    nextAddr = region.startAddress;
                    nextEndAddr = region.endAddress;
                    break;
                }
            }
        };

        u32 address;
        u32 endAddr;
        u32 nextAddr;
        u32 nextEndAddr;
        u32 index = _currentPosition;

        getAddress(index, address, endAddr, nextAddr, nextEndAddr);

        // First get the results from the file
        if (reinterpret_cast<Search<float> *>(_previousSearch)->_ReadResults(oldResultsUnkn, index, _maxResult))
        {
            SearchResult<float>     *result = reinterpret_cast<SearchResult<float>*>(_resultsP);
            ResultUnknownIter iter = oldResultsUnkn.begin();
            ResultUnknownIter end = oldResultsUnkn.end();
            u32 count = 1;

            for (; iter != end; ++iter, ++count)
            {
                float   newval = *reinterpret_cast<float *>(address);
                float   oldVal = iter->value;

                if (!std::isnan(newval) && (this->*_compare)(oldVal, newval))
                {
                    ++ResultCount;
                    result->address = address;
                    result->value = newval;
                    result->oldValue = oldVal;
                    ++result;

                    if (result >= _resultsEnd)
                        break;
                }

                address += _alignment;
                if (address >= endAddr)
                {
                    if (endAddr == nextEndAddr)
                        break;

                    address = nextAddr;
                    endAddr = nextEndAddr;
                }
            }

            _resultsP = result;

            // Update position
            _currentPosition += count;

            if (_currentPosition >= _previousSearch->ResultCount)
                return (true);
        }

        return (false);
    }
    /*
    template <>
    bool    Search<u8>::_SecondUnknownSearch(void)
    {
        std::vector<SearchResultUnknown<u32>> oldResultsUnkn;

        auto getAddress = [this](u32 index, u32 &addr, u32 &end, u32 &nextAddr, u32 &nextEndAddr)
        {
            Search<u8> *prev = reinterpret_cast<Search<u8> *>(_previousSearch);

            u64 offset = prev->_GetHeaderSize() + prev->_GetResultStructSize() * index;
            u32 size = prev->_GetResultStructSize();

            for (SRegion &region : prev->_regionsList)
            {
                if (offset >= region.fileOffset)
                {
                    addr = region.startAddress;
                    addr += (offset - region.fileOffset);
                    end = region.endAddress;
                    nextEndAddr = end;
                }
                else
                {
                    nextAddr = region.startAddress;
                    nextEndAddr = region.endAddress;
                    break;
                }
            }
        };

        u32 address;
        u32 endAddr;
        u32 nextAddr;
        u32 nextEndAddr;
        u32 index = _currentPosition;

        getAddress(index, address, endAddr, nextAddr, nextEndAddr);

        // First get the results from the file
        if (reinterpret_cast<Search<u8> *>(_previousSearch)->_ReadResults(oldResultsUnkn, index, _maxResult))
        {
            SearchResult<u8>     *result = reinterpret_cast<SearchResult<u8>*>(_resultsP);
            std::vector<SearchResultUnknown<u32>>::iterator iter = oldResultsUnkn.begin();
            std::vector<SearchResultUnknown<u32>>::iterator end = oldResultsUnkn.end();
            u32 count = 1;

            for (; iter != end; ++iter, ++count)
            {
                u32   newValBase = *reinterpret_cast<u32 *>(address);
                u32   oldValBase = iter->value;

                u8    newVal = newValBase;
                u8    oldVal = oldValBase;

                for (int i = 0; i < 4; i++)
                {
                    if ((this->*_compare)(oldVal, newVal))
                    {
                        ++ResultCount;
                        result->address = address + i;
                        result->value = newVal;
                        result->oldValue = oldVal;
                        ++result;

                        if (result >= _resultsEnd)
                            break;
                    }

                    int shift = 8 * (i + 1);
                    newVal = newValBase >> shift;
                    oldValBase = oldValBase >> shift;
                }
                
                address += 4;
                if (address >= endAddr)
                {
                    if (endAddr == nextEndAddr)
                        break;

                    address = nextAddr;
                    endAddr = nextEndAddr;
                }
            }

            _resultsP = result;

            // Update position
            _currentPosition += count;

            if (_currentPosition >= _previousSearch->ResultCount)
                return (true);
        }

        return (false);
    }

    template <>
    bool    Search<u16>::_SecondUnknownSearch(void)
    {
        std::vector<SearchResultUnknown<u32>> oldResultsUnkn;

        auto getAddress = [this](u32 index, u32 &addr, u32 &end, u32 &nextAddr, u32 &nextEndAddr)
        {
            Search<u16> *prev = reinterpret_cast<Search<u16> *>(_previousSearch);

            u64 offset = prev->_GetHeaderSize() + prev->_GetResultStructSize() * index;
            u32 size = prev->_GetResultStructSize();

            for (SRegion &region : prev->_regionsList)
            {
                if (offset >= region.fileOffset)
                {
                    addr = region.startAddress;
                    addr += (offset - region.fileOffset);
                    end = region.endAddress;
                    nextEndAddr = end;
                }
                else
                {
                    nextAddr = region.startAddress;
                    nextEndAddr = region.endAddress;
                    break;
                }
            }
        };

        u32 address;
        u32 endAddr;
        u32 nextAddr;
        u32 nextEndAddr;
        u32 index = _currentPosition;

        getAddress(index, address, endAddr, nextAddr, nextEndAddr);

        // First get the results from the file
        if (reinterpret_cast<Search<u8> *>(_previousSearch)->_ReadResults(oldResultsUnkn, index, _maxResult))
        {
            SearchResult<u8>     *result = reinterpret_cast<SearchResult<u8>*>(_resultsP);
            std::vector<SearchResultUnknown<u32>>::iterator iter = oldResultsUnkn.begin();
            std::vector<SearchResultUnknown<u32>>::iterator end = oldResultsUnkn.end();
            u32 count = 1;

            for (; iter != end; ++iter, ++count)
            {
                u32   newValBase = *reinterpret_cast<u32 *>(address);
                u32   oldValBase = iter->value;

                u8    newVal = newValBase;
                u8    oldVal = oldValBase;

                for (int i = 0; i < 4; i++)
                {
                    if ((this->*_compare)(oldVal, newVal))
                    {
                        ++ResultCount;
                        result->address = address + i;
                        result->value = newVal;
                        result->oldValue = oldVal;
                        ++result;

                        if (result >= _resultsEnd)
                            break;
                    }

                    int shift = 8 * (i + 1);
                    newVal = newValBase >> shift;
                    oldValBase = oldValBase >> shift;
                }

                address += 4;
                if (address >= endAddr)
                {
                    if (endAddr == nextEndAddr)
                        break;

                    address = nextAddr;
                    endAddr = nextEndAddr;
                }
            }

            _resultsP = result;

            // Update position
            _currentPosition += count;

            if (_currentPosition >= _previousSearch->ResultCount)
                return (true);
        }

        return (false);
    } */

    template <typename T>
    bool    Search<T>::_SubsidiarySearch(void)
    {
        std::vector<SearchResult<T>>        oldResults;
        u32 index = _currentPosition;

        // First get the results from the file
        if (reinterpret_cast<Search<T> *>(_previousSearch)->_ReadResults(oldResults, index, _maxResult))
        {
            SearchResult<T>     *result = reinterpret_cast<SearchResult<T>*>(_resultsP);
            ResultIter iter = oldResults.begin();
            ResultIter end = oldResults.end();
            u32 count = 1;

            for (; iter != end; ++iter, ++count)
            {
                u32 addr = iter->address;
                T   newval = *reinterpret_cast<T *>(addr);
                T   oldVal = iter->value;

                if ((this->*_compare)(oldVal, newval))
                {
                    ++ResultCount;
                    result->address = addr;
                    result->value = newval;
                    result->oldValue = oldVal;
                    ++result;

                    if (result >= _resultsEnd)
                        break;
                }
            }
            _resultsP = result;
            // Update position
            _currentPosition += count;
        }

        // Finish
        if (_currentPosition >= _previousSearch->ResultCount)
        {
            return (true);
        }

        return (false);
    }

    template <>
    bool    Search<float>::_SubsidiarySearch(void)
    {
        std::vector<SearchResult<float>>        oldResults;
        u32 index = _currentPosition;

        // First get the results from the file
        if (reinterpret_cast<Search<float> *>(_previousSearch)->_ReadResults(oldResults, index, _maxResult))
        {
            SearchResult<float>     *result = reinterpret_cast<SearchResult<float>*>(_resultsP);
            ResultIter iter = oldResults.begin();
            ResultIter end = oldResults.end();
            u32 count = 1;

            for (; iter != end; ++iter, ++count)
            {
                u32 addr = iter->address;
                float   newval = *reinterpret_cast<float *>(addr);
                float   oldVal = iter->value;

                if (!std::isnan(newval) && (this->*_compare)(oldVal, newval))
                {
                    ++ResultCount;
                    result->address = addr;
                    result->value = newval;
                    result->oldValue = oldVal;
                    ++result;

                    if (result >= _resultsEnd)
                        break;
                }
            }
            _resultsP = result;
            // Update position
            _currentPosition += count;
        }

        // Finish
        if (_currentPosition >= _previousSearch->ResultCount)
        {
            return (true);
        }

        return (false);
    }

    template<typename T>
    bool    Search<T>::_SearchInRange(void)
    {
        // First Search ?
        if (_previousSearch == nullptr)
        {
            u32 start = _currentPosition == 0 ? _startRange : _currentPosition;
            
            u32 end = start + std::min((u32)(_endRange - start), (u32)0x40000);

            if (_alignment != sizeof(T))
            {
                end -= sizeof(T);
            }

            if (Type == SearchType::ExactValue)
            {
                _FirstExactSearch(start, end);

                // Update position
                _currentPosition = start;
            }                
            else // Unknown value
            {           
                _FirstUnknownSearch(start, end);
                // Update position
                _currentPosition = start;
            }

            // Finish
            if (_currentPosition >= (_endRange - sizeof(T)))
            {
                return (true);
            }
        }
        // Subsidiary search
        else
        {  
            SearchFlags flags = reinterpret_cast<Search<T> *>(_previousSearch)->_flags;

            if (flags == FirstExact) return (_SecondExactSearch());
            if (flags == FirstUnknown) return (_SecondUnknownSearch());
            
            return (_SubsidiarySearch());            
        }

        return (false);
    }

    template<typename T>
    bool    Search<T>::DoSearch(void)
    {
        // Just started search ?
        if (_currentPosition == 0)
        {
            // Set flags / variables according to search parameters
            // At this step, _maxResult == pool size
            if (_previousSearch == nullptr)
            {
                if (Type == SearchType::Unknown)
                {
                    _flags = FirstUnknown;
                    _maxResult = (_maxResult / sizeof(SearchResultUnknown<T>)) - 1;
                    _resultsEnd = (void *)(((SearchResultUnknown<T> *)_resultsArray) + _maxResult);
                }
                else
                {
                    _flags = FirstExact;
                    _maxResult = (_maxResult / sizeof(SearchResultFirst<T>)) - 1;
                    _resultsEnd = (void *)((u32)_resultsArray + _maxResult * _GetResultStructSize());
                }
            }
            else
            {
                _maxResult = (_maxResult / sizeof(SearchResult<T>)) - 1;
                _resultsEnd = (void *)(((SearchResult<T> *)_resultsArray) + _maxResult);
            }

            _UpdateCompare();
            _WriteHeaderToFile();
            _clock.Restart();
        }

        bool    finished;

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
                _file.Flush();
            }
            else
            {
                if (_resultsP >= _resultsEnd)
                {
                    ResultsToFile();
                }
                    
            }
            return (finished);
        }
        // All regions search
        {
            // Start search
            if (_previousSearch == nullptr)
            {
                if (_currentRegion == -1)
                {                
                    _currentRegion = 0;
                    _startRange = _regionsList[0].startAddress;
                    _endRange = _regionsList[0].endAddress;
                    _regionsList[0].fileOffset = _GetHeaderSize();
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
                    ResultsToFile();
                    _currentRegion++;
                    _startRange = _regionsList[_currentRegion].startAddress;
                    _endRange = _regionsList[_currentRegion].endAddress;
                    _regionsList[_currentRegion].fileOffset = _file.Tell();
                    _currentPosition = _startRange;
                    svcFlushProcessDataCache(Process::GetHandle(), (void *)_startRange, _endRange - _startRange);
                }
                else
                {
                    ResultsToFile();
                    _startRange = _regionsList[0].startAddress;
                    _WriteHeaderToFile();
                    SearchTime = _clock.Restart();

                    // Free buffer
                    linearFree(_resultsArray);
                    _file.Flush();

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
        u8      alignment;
        u8      size;
        u8      valuesize;
        SearchFlags flags;
        u32     startRange;
        u32     endRange;        
        u32     resultcount;    
        u32     offset;
        u32     resultSize;        
        T       value;
        u32     regionCount;
        u8      padding[0x200];
    };

    template <typename T>
    bool    Search<T>::_WriteHeaderToFile(void)
    {
        u64     offset = _file.Tell();
        int     ret = 0;

        _file.Rewind();

        Header<T>   header = { 0 };

        header.type = (u8)Type;
        header.compare = (u8)Compare;
        header.alignment = _alignment;
        header.size = (u8)Size;
        header.valuesize = sizeof(T);
        header.flags = _flags;
        header.startRange = _startRange;
        header.endRange = _endRange;
        header.resultcount = ResultCount;
        header.offset = sizeof(Header<T>);
        header.resultSize = _flags == Subsidiary ? sizeof(SearchResult<T>) : (_flags == FirstExact ? sizeof(SearchResultFirst<T>) : sizeof(SearchResultUnknown<T>)); 
        header.value = _checkValue;
        header.regionCount = _regionsList.size();

        std::memcpy(header.padding, _regionsList.data(), _regionsList.size() * sizeof(SRegion));

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
        return (sizeof(Header<T>));
    }

    template <typename T>
    u32     Search<T>::_GetResultStructSize(void) const
    {
        if (_flags == FirstExact) return (sizeof(SearchResultFirst<T>));
        if (_flags == FirstUnknown) return (sizeof(SearchResultUnknown<T>));

        return (sizeof(SearchResult<T>));
    }

    template <typename T>
    bool    Search<T>::ResultsToFile(void)
    {
        int res = _file.Write(_resultsArray, (u32)(((u32)_resultsP - (u32)_resultsArray)));
        _resultsP = _resultsArray;
        return (res == 0);
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
    bool    Search<T>::_ReadResults(std::vector<SearchResultUnknown<T>> &out, u32 &index, u32 count)
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
    bool    Search<T>::_ReadResults(std::vector<SearchResultFirst<T>> &out, u32 &index, u32 count)
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
        
        _file.Flush();
    }

    template <typename T>
    bool    Search<T>::FetchResults(stringvector &address, stringvector &newval, stringvector &oldvalue, u32 index, int count)
    {
        auto getAddress = [this](u32 index, u32 &addr, u32 &end, u32 &nextAddr)
        {
            u64 offset = _GetHeaderSize() + _GetResultStructSize() * index;
            u32 size = _GetResultStructSize();

            for (SRegion &region : _regionsList)
            {
                if (offset >= region.fileOffset)
                {
                    addr = region.startAddress;
                    addr += (offset - region.fileOffset);// / size;
                    end = region.endAddress;
                    nextAddr = end + 1;
                }
                else
                {
                    nextAddr = region.startAddress;
                    break;
                }
            }
        };

        std::vector<SearchResult<T>>        newResults;
        std::vector<SearchResultFirst<T>>   newResultsFirst;
        std::vector<SearchResultUnknown<T>> newResultsUnkn;
        char   buffer[20] = {0};

        if (_flags == FirstExact) goto firstExact;
        if (_flags == FirstUnknown) goto firstUnknown;
        goto subsidiary;

        // FirstExact Search
    firstExact:

        if (!_ReadResults(newResultsFirst, index, count))
            return (false);

        for (int i = 0; i < newResultsFirst.size(); i++)
        {
            // Address
            sprintf(buffer, "%08X", newResultsFirst[i].address);
            address.push_back(buffer);

            // Value
            switch (Size)
            {
            case SearchSize::Bits8: sprintf(buffer, "%02X", newResultsFirst[i].value); break;
            case SearchSize::Bits16: sprintf(buffer, "%04X", newResultsFirst[i].value); break;
            case SearchSize::Bits32: sprintf(buffer, "%08X", newResultsFirst[i].value); break;
            case SearchSize::Bits64: sprintf(buffer, "%016llX", newResultsFirst[i].value); break;
            case SearchSize::FloatingPoint: sprintf(buffer, "%15.7f", newResultsFirst[i].value); break;
            case SearchSize::Double: sprintf(buffer, "%15.7g", newResultsFirst[i].value); break;
            }
            newval.push_back(buffer);
        }

        return (true);

        // FirstUnknown Search
    firstUnknown:

        u32 addr;
        u32 endAddr;
        u32 nextAddr;

        getAddress(index, addr, endAddr, nextAddr);

        if (!_ReadResults(newResultsUnkn, index, count))
            return (false);

        for (int i = 0; i < newResultsUnkn.size(); i++)
        {
            // Address
            sprintf(buffer, "%08X", addr);//newResultsUnkn[i].address);
            address.push_back(buffer);

            // Value
            switch (Size)
            {
            case SearchSize::Bits8: sprintf(buffer, "%02X", newResultsUnkn[i].value); break;
            case SearchSize::Bits16: sprintf(buffer, "%04X", newResultsUnkn[i].value); break;
            case SearchSize::Bits32: sprintf(buffer, "%08X", newResultsUnkn[i].value); break;
            case SearchSize::Bits64: sprintf(buffer, "%016llX", newResultsUnkn[i].value); break;
            case SearchSize::FloatingPoint: sprintf(buffer, "%15.7f", newResultsUnkn[i].value); break;
            case SearchSize::Double: sprintf(buffer, "%15.7g", newResultsUnkn[i].value); break;
            }
            newval.push_back(buffer);

            addr += _alignment;
            if (addr >= endAddr)
            {
                addr = nextAddr;
            }
        }

        return (true);

        // Subsidiary Search
    subsidiary:
    
        if (!_ReadResults(newResults, index, count))
            return (false);

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
            case SearchSize::FloatingPoint: sprintf(buffer, "%10f", newResults[i].value); break;
            case SearchSize::Double: sprintf(buffer, "%15.7g", newResults[i].value); break;
            }
            newval.push_back(buffer);

            // Old Value
            switch (Size)
            {
            case SearchSize::Bits8: sprintf(buffer, "%02X", newResults[i].oldValue); break;
            case SearchSize::Bits16: sprintf(buffer, "%04X", newResults[i].oldValue); break;
            case SearchSize::Bits32: sprintf(buffer, "%08X", newResults[i].oldValue); break;
            case SearchSize::Bits64: sprintf(buffer, "%016llX", newResults[i].oldValue); break;
            case SearchSize::FloatingPoint: sprintf(buffer, "%15.7f", newResults[i].oldValue); break;
            case SearchSize::Double: sprintf(buffer, "%15.7g", newResults[i].oldValue); break;
            }
            oldvalue.push_back(buffer);
        }

        return (true);
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

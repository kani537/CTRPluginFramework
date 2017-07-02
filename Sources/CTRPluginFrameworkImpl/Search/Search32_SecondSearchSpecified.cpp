#include "CTRPluginFrameworkImpl/Search/Search32.hpp"

namespace CTRPluginFramework
{
    void    Search32::SecondSearchSpecifiedU8(const std::vector<Results32>& data, SearchFlags compare, Results32WithOld* result)
    {
        u8 checkValue = _checkValue.U8;

        // If current search is specified
        if (IsSpecifiedSearch(_flags))
        {
            switch (compare)
            {
            case SearchFlags::Equal:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (EQ(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::NotEqual:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (NE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterThan:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (GT(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterOrEqual:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (GE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserThan:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (LT(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserOrEqual:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (LE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentBy:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (DB(oldValue, newValue, _checkValue.U8))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByLess:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (DBL(oldValue, newValue, _checkValue.U8))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByMore:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (DBM(oldValue, newValue, _checkValue.U8))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            }
        }
        // Else for unknown search
        else
        {
            switch (compare)
            {
            case SearchFlags::Equal:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (EQ(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::NotEqual:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (NE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterThan:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (GT(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterOrEqual:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (GE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserThan:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (LT(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserOrEqual:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (LE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentBy:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (DB(oldValue, newValue, _checkValue.U8))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByLess:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (DBL(oldValue, newValue, _checkValue.U8))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByMore:
            {
                for (Results32 res : data)
                {
                    u8 newValue = *(u8 *)res.address;
                    u8 oldValue = res.value.U8;

                    if (DBM(oldValue, newValue, _checkValue.U8))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            }

        }
    }

    void    Search32::SecondSearchSpecifiedU16(const std::vector<Results32>& data, SearchFlags compare, Results32WithOld* result)
    {
        u16 checkValue = _checkValue.U16;

        // If current search is specified
        if (IsSpecifiedSearch(_flags))
        {
            switch (compare)
            {
            case SearchFlags::Equal:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (EQ(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::NotEqual:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (NE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::GreaterThan:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (GT(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::GreaterOrEqual:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (GE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::LesserThan:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (LT(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::LesserOrEqual:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (LE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::DifferentBy:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (DB(oldValue, newValue, _checkValue.U16))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::DifferentByLess:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (DBL(oldValue, newValue, _checkValue.U16))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::DifferentByMore:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (DBM(oldValue, newValue, _checkValue.U16))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            }
        }

        // Else for unknown search
        else
        {
            switch (compare)
            {
            case SearchFlags::Equal:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (EQ(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::NotEqual:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (NE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::GreaterThan:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (GT(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::GreaterOrEqual:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (GE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::LesserThan:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (LT(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::LesserOrEqual:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (LE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::DifferentBy:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (DB(oldValue, newValue, _checkValue.U16))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::DifferentByLess:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (DBL(oldValue, newValue, _checkValue.U16))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            case SearchFlags::DifferentByMore:
            {
                for (Results32 res : data)
                {
                    u16 newValue = *(u16 *)res.address;
                    u16 oldValue = res.value.U16;

                    if (DBM(oldValue, newValue, _checkValue.U16))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress += 2;
                }
                break;
            }
            }
        }
    }

    void    Search32::SecondSearchSpecifiedU32(const std::vector<Results32>& data, SearchFlags compare, Results32WithOld* result)
    {
        u32 checkValue = _checkValue.U32;

        // If current search is specified
        if (IsSpecifiedSearch(_flags))
        {
            switch (compare)
            {
            case SearchFlags::Equal:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;


                    if (EQ(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::NotEqual:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (NE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterThan:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (GT(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterOrEqual:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (GE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserThan:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (LT(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserOrEqual:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (LE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentBy:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (DB(oldValue, newValue, _checkValue.U32))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByLess:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (DBL(oldValue, newValue, _checkValue.U32))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByMore:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (DBM(oldValue, newValue, _checkValue.U32))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            }
        }

        // Else for unknown search
        else
        {
            switch (compare)
            {
            case SearchFlags::Equal:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;


                    if (EQ(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::NotEqual:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (NE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterThan:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (GT(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterOrEqual:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (GE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserThan:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (LT(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserOrEqual:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (LE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentBy:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (DB(oldValue, newValue, _checkValue.U32))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByLess:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (DBL(oldValue, newValue, _checkValue.U32))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByMore:
            {
                for (Results32 res : data)
                {
                    u32 newValue = *(u32 *)res.address;
                    u32 oldValue = res.value.U32;

                    if (DBM(oldValue, newValue, _checkValue.U32))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            }
        }
    }

    void    Search32::SecondSearchSpecifiedFloat(const std::vector<Results32>& data, SearchFlags compare, Results32WithOld* result)
    {
        float checkValue = _checkValue.Float;

        // If current search is specified
        if (IsSpecifiedSearch(_flags))
        {
            switch (compare)
            {
            case SearchFlags::Equal:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (EQ(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::NotEqual:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (NE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterThan:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (GT(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterOrEqual:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (GE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserThan:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (LT(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserOrEqual:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (LE(checkValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentBy:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (DB(oldValue, newValue, _checkValue.Float))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByLess:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (DBL(oldValue, newValue, _checkValue.Float))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByMore:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (DBM(oldValue, newValue, _checkValue.Float))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            }
        }

        // Else for unknown search
        else
        {
            switch (compare)
            {
            case SearchFlags::Equal:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (EQ(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::NotEqual:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (NE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterThan:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (GT(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::GreaterOrEqual:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (GE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserThan:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (LT(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::LesserOrEqual:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (LE(oldValue, newValue))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentBy:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (DB(oldValue, newValue, _checkValue.Float))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByLess:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (DBL(oldValue, newValue, _checkValue.Float))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            case SearchFlags::DifferentByMore:
            {
                for (Results32 res : data)
                {
                    float newValue = *(float *)res.address;
                    float oldValue = res.value.Float;

                    if (DBM(oldValue, newValue, _checkValue.Float))
                    {
                        *result++ = { res.address, newValue, oldValue };
                        _resultsInPool++;
                        ResultsCount++;
                    }
                    _currentAddress++;
                }
                break;
            }
            }
        }
    }
}

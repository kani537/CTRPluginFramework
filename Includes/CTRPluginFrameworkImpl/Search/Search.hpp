#ifndef CTRPLUGINFRAMEWORKIMPL_SEARCH_HPP
#define CTRPLUGINFRAMEWORKIMPL_SEARCH_HPP

#include "types.h"
#include "CTRPluginFramework/System/Clock.hpp"
#include "CTRPluginFramework/System/File.hpp"

#include <vector>
#include <string>

namespace CTRPluginFramework
{
    union Types32_u
    {
        u32     U32;
        u16     U16;
        u8      U8;
        float   Float;
    };

    union Types64_u
    {
        u64     U64;
        double  Double;
    };

    union  Bytes32
    {
        u32 U32;
        struct
        {
            u8 b1;
            u8 b2;
            u8 b3;
            u8 b4;
        };
    };

    union  Short32
    {
        u32 U32;
        struct
        {
            u16 s1;
            u16 s2;
        };
    };

    enum class SearchFlags
    {
        U8 = 1,
        U16 = 1 << 1,
        U32 = 1 << 2,
        U64 = 1 << 3,
        Float = 1 << 4,
        Double = 1 << 5,

        Unknown = 1 << 6,
        First = 1 << 7,
        Second = 1 << 8,

        Equal = 1 << 9,
        NotEqual = 1 << 10,
        GreaterThan = 1 << 11,
        GreaterOrEqual = 1 << 12,
        LesserThan = 1 << 13,
        LesserOrEqual = 1 << 14,
        DifferentBy = 1 << 15,
        DifferentByLess = 1 << 16,
        DifferentByMore = 1 << 17

    };

#define EQ(x, y) (x == y)
#define NE(x, y) (x != y)
#define GT(x, y) (x > y)
#define GE(x, y) (x >= y)
#define LT(x, y) (x < y)
#define LE(x, y) (x <= y)
#define DB(x, y, z) (x == (y + z) || x == (y - z))
#define DBL(x, y, z) ((x - z) < y || y < (x + z))
#define DBM(x, y, z) (y < (x - z) || y > (x + z))

#define U8_First (u32)(SearchFlags::U8 | SearchFlags::First)
#define U16_First (u32)(SearchFlags::U8 | SearchFlags::First)
#define U32_First (u32)(SearchFlags::U8 | SearchFlags::First)
#define U64_First (u32)(SearchFlags::U8 | SearchFlags::First)
#define Float_First (u32)(SearchFlags::U8 | SearchFlags::First)
#define Double_First (u32)(SearchFlags::U8 | SearchFlags::First)

#define U8_First_Unknown (u32)(SearchFlags::U8 | SearchFlags::First | SearchFlags::Unknown)
#define U16_First_Unknown (u32)(SearchFlags::U8 | SearchFlags::First | SearchFlags::Unknown)
#define U32_First_Unknown (u32)(SearchFlags::U8 | SearchFlags::First | SearchFlags::Unknown)
#define U64_First_Unknown (u32)(SearchFlags::U8 | SearchFlags::First | SearchFlags::Unknown)
#define Float_First_Unknown (u32)(SearchFlags::U8 | SearchFlags::First | SearchFlags::Unknown)
#define Double_First_Unknown (u32)(SearchFlags::U8 | SearchFlags::First | SearchFlags::Unknown)

#define IsUnknownSearch(flags) (flags & (u32)SearchFlags::Unknown)
#define IsSpecifiedSearch(flags) !IsUnknownSearch(flags)
#define IsFirstSearch(flags) (flags & (u32)SearchFlags::First)
#define IsSecondSearch(flags) (flags & (u32)SearchFlags::Second)
#define IsSubsidiarySearch(flags) (!(flags & (u32)(SearchFlags::First | SearchFlags::Second)))

#define TypeFlags(flags) ((u32)(flags) & 0x3F)
#define CompareFlags(flags) ((u32)(flags) & 0x3FE00)

    struct Results32
    {
        u32         address;
        Types32_u   value;
    };

    struct Results32WithOld
    {
        u32         address;
        Types32_u   newValue;
        Types32_u   oldValue;
    };

    struct Results64
    {
        u32         address;
        Types64_u   value;
    };

    struct Results64WithOld
    {
        u32         address;
        Types64_u   newValue;
        Types64_u   oldValue;
    };

    struct Region
    {
        u32     startAddress;
        u32     endAddress;
    };

    struct RegionOnFile
    {
        u32     startAddress;
        u32     endAddress;
        u64     fileOffset;
        u32     nbResults;
    };

    struct SearchError
    {
        bool    file : 1;
        bool    pool : 1;

        u32     fsError;
    };

    class Search;
    struct SearchParameters
    {
        u32                     flags;
        Types32_u               value32;
        Types64_u               value64;
        bool                    fullMemory;
        std::vector<Region>     ranges;
        Search                  *previous;
    };

    struct Header
    {
        u32     flags;
        u32     nbRegions;
        RegionOnFile    regions[50];
    };
}

#endif

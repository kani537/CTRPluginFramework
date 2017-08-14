#ifndef CTRPLUGINFRAMEWORKIMPL_ACTIONREPLAY_ARCODE_HPP
#define CTRPLUGINFRAMEWORKIMPL_ACTIONREPLAY_ARCODE_HPP

#include "types.h"
#include <vector>

namespace CTRPluginFramework
{
    class ARCode
    {
    public:

        virtual ~ARCode(void) {}

        u8      Type;
        u32     Left;
        u32     Right;
        u8      *Data; ///< Pointer to data for E code

        virtual std::string ToString(void);

        ARCode(u8 type, u32 left, u32 right) :
            Type(type), Left(left), Right(right), Data(nullptr) {}
        ARCode(const std::string &line, bool &error);
    };

    using ARCodeVector = std::vector<ARCode>;
}

#endif
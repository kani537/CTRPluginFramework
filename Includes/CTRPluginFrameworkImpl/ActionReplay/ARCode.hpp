#ifndef CTRPLUGINFRAMEWORKIMPL_ACTIONREPLAY_ARCODE_HPP
#define CTRPLUGINFRAMEWORKIMPL_ACTIONREPLAY_ARCODE_HPP

#include "types.h"
#include <string>
#include <vector>

namespace CTRPluginFramework
{
    namespace ActionReplayPriv
    {
        u32     Str2U32(const std::string &str, bool &error);
    }

    class ARCode
    {
    public:

        virtual ~ARCode(void) {}

        u8      Type;
        u32     Left;
        u32     Right;
        u8      *Data; ///< Pointer to data for E code

        virtual std::string ToString(void) const;

        ARCode(u8 type, u32 left, u32 right) :
            Type(type), Left(left), Right(right), Data(nullptr) {}
        ARCode(const std::string &line, bool &error);
    };

    using ARCodeVector = std::vector<ARCode>;

    struct ARCodeContext
    {
        bool            hasError;   ///< True if any of the codes has an unrecognized char
        std::string     data;       ///< Original data in case of error
        u32             storage[2]; ///< Storage for this code
        ARCodeVector    codes;      ///< List of all codes

        void            Clear(void);
    };

    class MenuFolderImpl;
    void    ActionReplay_LoadCodes(MenuFolderImpl *dst);
}

#endif

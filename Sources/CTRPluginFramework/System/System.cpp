#include "types.h"
#include "3DS.h"
#include "CTRPluginFramework/System/System.hpp"
#include "CTRPluginFrameworkImpl/System/SystemImpl.hpp"

namespace CTRPluginFramework
{
    bool    System::IsNew3DS(void)
    {
        return (SystemImpl::_isNew3DS);
    }
}
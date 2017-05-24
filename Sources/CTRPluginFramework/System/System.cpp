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

    Language    System::GetSystemLanguage(void)
    {
        return (static_cast<Language>(SystemImpl::_language));
    }

    bool    System::IsConnectedToInternet(void)
    {
        u32 out = 0;

        ACU_GetWifiStatus(&out);

        return (out != 0);
    }
}

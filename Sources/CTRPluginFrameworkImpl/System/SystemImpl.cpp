#include "types.h"
#include "3DS.h"
#include "CTRPluginFrameworkImpl/System/SystemImpl.hpp"

namespace CTRPluginFramework
{
    bool    SystemImpl::IsNew3DS = false;
    bool    SystemImpl::IsLoaderNTR = false;
    u32     SystemImpl::IoBaseLCD = 0;
    u32     SystemImpl::IoBasePAD = 0;
    u32     SystemImpl::IoBasePDC = 0;
    u32     SystemImpl::CFWVersion = 0;
    u32     SystemImpl::RosalinaHotkey = 0;
    u8      SystemImpl::Language = CFG_LANGUAGE_EN;

    extern "C" u32     g_KProcessPIDOffset;
    extern "C" u32     g_KProcessKernelFlagsOffset;

    void    SystemImpl::Initialize(void)
    {
        bool isNew3DS = false;

        APT_CheckNew3DS(&isNew3DS);
        IsNew3DS = isNew3DS;
        if (isNew3DS)
        {
            IoBaseLCD = 0xFFFC4000;
            IoBasePAD = 0xFFFC2000;
            IoBasePDC = 0xFFFBC000;
            g_KProcessPIDOffset = 0xBC;
            g_KProcessKernelFlagsOffset = 0xB0;
        }
        else
        {
            IoBaseLCD = 0xFFFC8000;
            IoBasePAD = 0xFFFC6000;
            IoBasePDC = 0xFFFC0000;
            g_KProcessPIDOffset = 0xB4;
            g_KProcessKernelFlagsOffset = 0xA8;
        }

        s64 out = 0;

        if (R_SUCCEEDED(svcGetSystemInfo(&out, 0x10000, 0)))
        {
            CFWVersion = static_cast<u32>(out);
            if (R_SUCCEEDED(svcGetSystemInfo(&out, 0x10000, 0x101)))
                RosalinaHotkey = static_cast<u32>(out);
        }
        else
            CFWVersion = 0; ///< Unknown

        // Get System's language
        CFGU_GetSystemLanguage(&Language);
    }
}

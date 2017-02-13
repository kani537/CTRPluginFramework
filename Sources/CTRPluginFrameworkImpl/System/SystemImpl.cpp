#include "types.h"
#include "3DS.h"
#include "CTRPluginFrameworkImpl/System/SystemImpl.hpp"

namespace CTRPluginFramework
{
    bool    SystemImpl::_isInit = false;
    bool    SystemImpl::_isNew3DS = false;
    u32     SystemImpl::_IOBaseLCD = 0;
    u32     SystemImpl::_IOBasePAD = 0;
    u32     SystemImpl::_IOBasePDC = 0;
    extern "C" u32     g_KProcessPIDOffset;

    void    SystemImpl::Initialize(void)
    {
        if (_isInit)
            return;

        bool isNew3DS = false;
        
        srvInit();
        APT_CheckNew3DS(&isNew3DS);
        _isNew3DS = isNew3DS;
        if (isNew3DS)
        {
            _IOBaseLCD = 0xFFFC4000;
            _IOBasePAD = 0xFFFC2000;
            _IOBasePDC = 0xFFFBC000;
            g_KProcessPIDOffset = 0xBC;
        }
        else
        {
            _IOBaseLCD = 0xFFFC8000;
            _IOBasePAD = 0xFFFC6000;
            _IOBasePDC = 0xFFFC0000;
            g_KProcessPIDOffset = 0xB4;
        }
        _isInit = true;
    }

    u32     SystemImpl::GetIOBaseLCD(void)
    {
        return (_IOBaseLCD);
    }

    u32     SystemImpl::GetIOBasePAD(void)
    {
        return (_IOBasePAD);
    }

    u32     SystemImpl::GetIOBasePDC(void)
    {
        return (_IOBasePDC);
    }
}
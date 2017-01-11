#include "CTRPluginFramework.hpp"
#include "3DS.h"

namespace CTRPluginFramework
{
    bool    System::_isInit = false;
    bool    System::_isNew3DS = false;
    u32     System::_IOBaseLCD = 0;
    u32     System::_IOBasePAD = 0;
    u32     System::_IOBasePDC = 0;

    void    System::Initialize(void)
    {
        if (_isInit)
            return;

        bool isNew3DS = false;
        
        APT_CheckNew3DS(&isNew3DS);
        _isNew3DS = isNew3DS;
        if (isNew3DS)
        {
            _IOBaseLCD = 0xFFFC4000;
            _IOBasePAD = 0xFFFC2000;
            _IOBasePDC = 0xFFFBC000;
        }
        else
        {
            _IOBaseLCD = 0xFFFC8000;
            _IOBasePAD = 0xFFFC6000;
            _IOBasePDC = 0xFFFC0000;
        }
        _isInit = true;
        Process::Initialize(true);
    }

    u32     System::GetIOBaseLCD(void)
    {
        return (_IOBaseLCD);
    }

    u32     System::GetIOBasePAD(void)
    {
        return (_IOBasePAD);
    }

    u32     System::GetIOBasePDC(void)
    {
        return (_IOBasePDC);
    }

    bool    System::IsNew3DS(void)
    {
        return (_isNew3DS);
    }
}
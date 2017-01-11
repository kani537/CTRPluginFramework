#include "System.hpp"
#include "ctrulib/services/apt.h"
#include "ctrulib/svc.h"
#include "libntrplg/sharedfunc.h"
namespace CTRPluginFramework
{
    bool    System::_isInit = false;
    bool    System::_isNew3DS = false;
    u32     System::_IOBaseLCD = 0;
    u32     System::_IOBasePAD = 0;
    u32     System::_IOBasePDC = 0;

    void gfxFillColor(u32 fillcolor)
    {
        u32 i;

        u32 IoBaseLcd = System::GetIOBaseLCD();
        for (i = 0; i < 0x64; ++i )
        {
            *(u32 *)(IoBaseLcd + 0x204) = fillcolor;
            *(u32 *)(IoBaseLcd + 0xA04) = fillcolor;
            svcSleepThread(5000000); // 0.005 second
        }
        *(u32*)(IoBaseLcd + 0x204) = 0;
        *(u32*)(IoBaseLcd + 0xA04) = 0;
    }

    void    Check(void)
    {
        if (System::IsNew3DS())
            gfxFillColor(0x100FF00);
        else
            gfxFillColor(0x10000FF);
    }

    void    System::Initialize(void)
    {
        if (_isInit)
            return;

        bool isNew3DS = false;
        //aptInit();
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
        Check();
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
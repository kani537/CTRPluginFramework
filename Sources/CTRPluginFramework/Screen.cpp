#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include <new>

namespace CTRPluginFramework
{
    static  char    topScreenMemory[sizeof(Screen)];
    static  char    bottomScreenMemory[sizeof(Screen)];

    Screen *Screen::Top = NULL;
    Screen  *Screen::Bottom = NULL;

    Screen::Screen(u32 lcdSetupInfo, u32 fillColorAddress) : 
    _LCDSetup(reinterpret_cast<LCDFamebufferSetup *>(lcdSetupInfo)),
    _FillColor((u32 *)fillColorAddress),
    _isInit(true)
    {

    }

    void    Screen::Initialize(void)
    {
        Screen::Top = new Screen(System::GetIOBasePDC() + 0x400, System::GetIOBaseLCD() + 0x204);
        Screen::Bottom = new Screen(System::GetIOBasePDC() + 0x500, System::GetIOBaseLCD() + 0xA04);
    }

    void    Screen::Flash(Color &color)
    {
        if (!_isInit)
            return;
        u32     fillColor = (color.ToU32() & 0xFFFFFF) | 0x01000000;

        for (int i = 0; i < 0x64; i++)
        {
            *(_FillColor) = fillColor;
            svcSleepThread(5000000); // 0.005 second
        }
        *(_FillColor) = 0;
    }
}
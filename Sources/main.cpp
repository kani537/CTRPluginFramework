#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include <stdlib.h>

using namespace CTRPluginFramework;

int    main(int arg)
{   
    // Wait for the game to be launched
    //svcSleepThread(0x100000000);   
    //initSystem();   
    System::Initialize();
    Screen::Initialize();
    svcSleepThread(0x50000000);

    int     i = 0;

    Color y = Color(255, 255, 0);
    Color z = Color(0, 255, 255);
    Color red = Color(255, 0, 0);
    Color green = Color(0, 255, 0);

    while (i++ < 1)
    {
        Screen::Top->Flash(i % 2 ? y : z);
        Screen::Bottom->Flash(i % 2 ? z : y);

        svcSleepThread(0x50000000);
    }

    char *p = nullptr;

    p = new char(0x1000);
    if (p == nullptr)
        Screen::Top->Flash(red);
    else
    {
        Screen::Top->Flash(green);
        for (i = 0; i < 0x1000; i++)
            *p++ = 1;
    }
    return (0);
}


#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include <stdlib.h>

using namespace CTRPluginFramework;

int    main(u64 titleId, char *processName)
{   
    svcSleepThread(0x50000000);

    int     i = 0;

    Color y = Color::Yellow;
    Color z = Color(0, 255, 255);
    Color red = Color(255, 0, 0);
    Color green = Color(0, 255, 0);
    Color magenta = Color(255, 0, 255);

    if (titleId == 0x0004000000033600)
        Screen::Top->Flash(magenta);

    while (i++ < 1)
    {
        Screen::Top->Flash(i % 2 ? y : z);
        Screen::Bottom->Flash(i % 2 ? z : y);

        svcSleepThread(0x50000000);
    }

    return (0);
}


#include <3DS.h>

void    __appExit(void)
{
	// Exit services
	hidExit();
    cfguExit();
    fsExit();
    amExit();
    acExit();
    srvExit();
}

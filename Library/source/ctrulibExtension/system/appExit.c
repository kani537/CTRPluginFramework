#include <3ds.h>

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

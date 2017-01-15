#include <3DS.h>

void __attribute__((weak)) __appExit(void)
{
	// Exit services
    //gfxExit();
    //gspExit();
	sdmcExit();
	fsExit();

	hidExit();
	//aptExit();
	srvExit();
}

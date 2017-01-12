#include <3DS.h>

void __attribute__((weak)) __appExit(void)
{
	// Exit services
    
	//sdmcExit();
	//fsExit();

	//hidExit();
	//aptExit();
	srvExit();
}

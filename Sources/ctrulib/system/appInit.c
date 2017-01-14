#include <3DS.h>

void __attribute__((weak)) __appInit(void) {
	// Initialize services
	srvInit();
	//aptInit();
	hidInit();

	fsInit();
	sdmcInit();
    //gfxInit();
    //gspInit();
}

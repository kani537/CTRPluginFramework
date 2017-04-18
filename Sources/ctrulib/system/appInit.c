#include <3DS.h>

void __attribute__((weak)) __appInit(void)
{
	//aptInit();
	hidInit();

	fsInit();
    fontEnsureMapped();
	//sdmcInit();
}

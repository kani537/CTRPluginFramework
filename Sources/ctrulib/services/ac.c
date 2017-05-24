#include <stdlib.h>
#include <types.h>
#include <ctrulib/result.h>
#include <ctrulib/svc.h>
#include <ctrulib/srv.h>
#include <ctrulib/synchronization.h>
#include <ctrulib/services/ac.h>
#include <ctrulib/ipc.h>

static Handle acHandle;
static int acRefCount;

Result acInit(void)
{
	Result ret;

	if (AtomicPostIncrement(&acRefCount)) return 0;

	ret = srvGetServiceHandle(&acHandle, "ac:u");
	if(R_FAILED(ret)) ret = srvGetServiceHandle(&acHandle, "ac:i");
	if(R_FAILED(ret)) AtomicDecrement(&acRefCount);

	return ret;
}

void acExit(void)
{
	if (AtomicDecrement(&acRefCount)) return;
	svcCloseHandle(acHandle);
}

Result acWaitInternetConnection(void)
{
	Result ret = 0;
	u32 status = 0;
	while(R_SUCCEEDED(ret = ACU_GetWifiStatus(&status)) && status == 0);
	return ret;
}

Result ACU_GetWifiStatus(u32 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,0,0); // 0x000D0000

	if(R_FAILED(ret = svcSendSyncRequest(acHandle)))return ret;

	*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}
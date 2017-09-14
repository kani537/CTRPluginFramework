#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <ctrulib/result.h>
#include <ctrulib/svc.h>
#include <ctrulib/srv.h>
#include <ctrulib/synchronization.h>
#include <ctrulib/services/gspgpu.h>
#include <ctrulib/ipc.h>
#include <ctrulib/thread.h>
#include "ctrulib/allocator/mappable.h"

#define GSP_EVENT_STACK_SIZE 0x1000

Handle gspGpuHandle;
static int gspRefCount;

static s32 gspLastEvent = -1;
static LightEvent gspEvents[GSPGPU_EVENT_MAX];
static vu32 gspEventCounts[GSPGPU_EVENT_MAX];
static ThreadFunc gspEventCb[GSPGPU_EVENT_MAX];
static void* gspEventCbData[GSPGPU_EVENT_MAX];
static bool gspEventCbOneShot[GSPGPU_EVENT_MAX];
static volatile bool gspRunEvents;
static Thread gspEventThread;
static u8 gfxThreadID;
static u8 *gfxSharedMemory;

static vu8* gspEventData;
Handle gspEvent, gspSharedMemHandle;
static void gspEventThreadMain(u32 arg);

Handle gspThreadEventHandle;
static char gspThreadEventStack[0x1000] = {0};

Handle __sync_get_arbiter(void);

Result gspInit(void)
{
	Result res=0;
	if (AtomicPostIncrement(&gspRefCount))
        return 0;
	res = srvGetServiceHandle(&gspGpuHandle, "gsp::Gpu");
	if (R_FAILED(res))
        AtomicDecrement(&gspRefCount);

    gfxSharedMemory = (u8 *)mappableAlloc(0x1000);

    // Setup our gsp shared mem section
    svcCreateEvent(&gspEvent, RESET_ONESHOT);
    GSPGPU_RegisterInterruptRelayQueue(gspEvent, 0x1, &gspSharedMemHandle, &gfxThreadID);
    svcMapMemoryBlock(gspSharedMemHandle, (u32)gfxSharedMemory, 0x3, 0x10000000);

    // Initialize even handler
    gspInitEventHandler(gspEvent, (vu8*)gfxSharedMemory, gfxThreadID);

	return res;
}

void gspExit(void)
{
	if (AtomicDecrement(&gspRefCount)) return;

    // Exit event handler
    gspExitEventHandler();

    // Unmap GSP shared mem
    svcUnmapMemoryBlock(gspSharedMemHandle, (u32)gfxSharedMemory);

    svcCloseHandle(gspSharedMemHandle);
    if (gfxSharedMemory != NULL)
    {
        mappableFree(gfxSharedMemory);
        gfxSharedMemory = NULL;
    }

    svcCloseHandle(gspEvent);

	svcCloseHandle(gspGpuHandle);
}

void gspSetEventCallback(GSPGPU_Event id, ThreadFunc cb, void* data, bool oneShot)
{
	if(id>= GSPGPU_EVENT_MAX)return;

	gspEventCb[id] = cb;
	gspEventCbData[id] = data;
	gspEventCbOneShot[id] = oneShot;
}

Result gspInitEventHandler(Handle _gspEvent, vu8* _gspSharedMem, u8 gspThreadId)
{
	// Initialize events
	int i;
	//for (i = 0; i < GSPGPU_EVENT_MAX; i ++)
		LightEvent_Init(&gspEvents[2], RESET_STICKY);
        LightEvent_Init(&gspEvents[3], RESET_STICKY);

	// Start event thread
	gspEvent = _gspEvent;
	gspEventData = _gspSharedMem + gspThreadId*0x40;
	gspRunEvents = true;
	svcCreateThread(&gspThreadEventHandle, gspEventThreadMain, 0, (u32*)&gspThreadEventStack[0x1000], 0x3F, -2);
	//gspEventThread = threadCreate(gspEventThreadMain, gspThreadEventStack, 0x1000, 0x31, -2, true);
	//gspThreadEventHandle = threadGetHandle(gspEventThread);
	return 0;
}

void gspExitEventHandler(void)
{
	// Stop event thread
	gspRunEvents = false;
	svcSignalEvent(gspEvent);
	svcWaitSynchronization(gspThreadEventHandle, U64_MAX);
	//threadJoin(gspEventThread, U64_MAX);
}

bool   IsGamePrior(void);
bool   IsPaused(void);

void gspWaitForEvent(GSPGPU_Event id, bool nextEvent)
{
    if (!IsPaused())
        return;
	if(id>= GSPGPU_EVENT_MAX)return;

	if (nextEvent)
		LightEvent_Clear(&gspEvents[id]);
	LightEvent_Wait(&gspEvents[id]);
	if (!nextEvent)
		LightEvent_Clear(&gspEvents[id]);
}

GSPGPU_Event gspWaitForAnyEvent(void)
{
    return;
	s32 x;
	do
	{
		do
		{
			x = __ldrex(&gspLastEvent);
			if (x < 0)
			{
				__clrex();
				break;
			}
		} while (__strex(&gspLastEvent, -1));
		if (x < 0)
			svcArbitrateAddress(__sync_get_arbiter(), (u32)&gspLastEvent, ARBITRATION_WAIT_IF_LESS_THAN, 0, 0);
	} while (x < 0);
	return (GSPGPU_Event)x;
}

static int popInterrupt()
{
	int curEvt;
	bool strexFailed;
	do {
		union {
			struct {
				u8 cur;
				u8 count;
				u8 err;
				u8 unused;
			};
			u32 as_u32;
		} header;

		// Do a load on all header fields as an atomic unit
		header.as_u32 = __ldrex((s32*)gspEventData);

		if (__builtin_expect(header.count == 0, 0) || !IsPaused()) {
			__clrex();
			return -1;
		}

		curEvt = gspEventData[0xC + header.cur];

		header.cur += 1;
		if (header.cur >= 0x34) header.cur -= 0x34;
		header.count -= 1;
		header.err = 0; // Should this really be set?

		strexFailed = __strex((s32*)gspEventData, header.as_u32);

	} while (__builtin_expect(strexFailed, 0));

	return curEvt;
}

void gspEventThreadMain(u32 arg)
{
	while (gspRunEvents)
	{
		svcWaitSynchronization(gspEvent, U64_MAX);
		svcClearEvent(gspEvent);

		while (true)
		{
			int curEvt = popInterrupt();

			if (curEvt == -1)
				break;

			if (curEvt == 2 || curEvt == 3 )// < GSPGPU_EVENT_MAX)
			{
				LightEvent_Signal(&gspEvents[curEvt]);
				gspEventCounts[curEvt]++;
			}
		}
	}
	svcExitThread();
}

//essentially : get commandIndex and totalCommands, calculate offset of new command, copy command and update totalCommands
//use LDREX/STREX because this data may also be accessed by the GSP module and we don't want to break stuff
//(mostly, we could overwrite the buffer header with wrong data and make the GSP module reexecute old commands)
Result gspSubmitGxCommand(u32* sharedGspCmdBuf, u32 gxCommand[0x8])
{
	if(!sharedGspCmdBuf || !gxCommand)return -1;

	u32 cmdBufHeader = __ldrex((s32*)sharedGspCmdBuf);

	u8 commandIndex=cmdBufHeader&0xFF;
	u8 totalCommands=(cmdBufHeader>>8)&0xFF;

	if(totalCommands>=15)return -2;

	u8 nextCmd=(commandIndex+totalCommands)%15; //there are 15 command slots
	u32* dst=&sharedGspCmdBuf[8*(1+nextCmd)];
	memcpy(dst, gxCommand, 0x20);

	__dsb();
	totalCommands++;
	cmdBufHeader=((cmdBufHeader)&0xFFFF00FF)|(((u32)totalCommands)<<8);

	while(1)
	{
		if (!__strex((s32*)sharedGspCmdBuf, cmdBufHeader)) break;

		cmdBufHeader = __ldrex((s32*)sharedGspCmdBuf);
		totalCommands=((cmdBufHeader&0xFF00)>>8)+1;
		cmdBufHeader=((cmdBufHeader)&0xFFFF00FF)|((totalCommands<<8)&0xFF00);
	}

	if(totalCommands==1)return GSPGPU_TriggerCmdReqQueue();
	return 0;
}

Result GSPGPU_WriteHWRegs(u32 regAddr, u32* data, u8 size)
{
	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x1,2,2); // 0x10082
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[3]=IPC_Desc_StaticBuffer(size, 0);
	cmdbuf[4]=(u32)data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_WriteHWRegsWithMask(u32 regAddr, u32* data, u8 datasize, u32* maskdata, u8 masksize)
{
	if(datasize>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x2,2,4); // 0x20084
	cmdbuf[1]=regAddr;
	cmdbuf[2]=datasize;
	cmdbuf[3]=IPC_Desc_StaticBuffer(datasize, 0);
	cmdbuf[4]=(u32)data;
	cmdbuf[5]=IPC_Desc_StaticBuffer(masksize, 1);
	cmdbuf[6]=(u32)maskdata;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ReadHWRegs(u32 regAddr, u32* data, u8 size)
{
	if(size>0x80 || !data)return -1;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x4,2,0); // 0x40080
	cmdbuf[1]=regAddr;
	cmdbuf[2]=size;
	cmdbuf[0x40]=IPC_Desc_StaticBuffer(size, 0);
	cmdbuf[0x40+1]=(u32)data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetBufferSwap(u32 screenid, GSPGPU_FramebufferInfo*framebufinfo)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,8,0); // 0x50200
	cmdbuf[1] = screenid;
	memcpy(&cmdbuf[2], framebufinfo, sizeof(GSPGPU_FramebufferInfo));

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_FlushDataCache(const void* adr, u32 size)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x8,2,2); // 0x80082
	cmdbuf[1]=(u32)adr;
	cmdbuf[2]=size;
	cmdbuf[3]=IPC_Desc_SharedHandles(1);
	cmdbuf[4]=CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_InvalidateDataCache(const void* adr, u32 size)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,2,2); // 0x90082
	cmdbuf[1] = (u32)adr;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_SetLcdForceBlack(u8 flags)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0xB,1,0); // 0xB0040
	cmdbuf[1]=flags;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_TriggerCmdReqQueue(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0xC,0,0); // 0xC0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_RegisterInterruptRelayQueue(Handle eventHandle, u32 flags, Handle* outMemHandle, u8* threadID)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x13,1,2); // 0x130042
	cmdbuf[1]=flags;
	cmdbuf[2]=IPC_Desc_SharedHandles(1);
	cmdbuf[3]=eventHandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	if(threadID)*threadID=cmdbuf[2] & 0xFF;
	if(outMemHandle)*outMemHandle=cmdbuf[4];

	return cmdbuf[1];
}

Result GSPGPU_UnregisterInterruptRelayQueue(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x14,0,0); // 0x140000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_AcquireRight(u8 flags)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x16,1,2); // 0x160042
	cmdbuf[1]=flags;
	cmdbuf[2]=IPC_Desc_SharedHandles(1);
	cmdbuf[3]=CUR_PROCESS_HANDLE;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ReleaseRight(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x17,0,0); // 0x170000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_ImportDisplayCaptureInfo(GSPGPU_CaptureInfo*captureinfo)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x18,0,0); // 0x180000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret)) memcpy(captureinfo, &cmdbuf[2], 0x20);

	return ret;
}

Result GSPGPU_SaveVramSysArea(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x19,0,0); // 0x190000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}

Result GSPGPU_RestoreVramSysArea(void)
{
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=IPC_MakeHeader(0x1A,0,0); // 0x1A0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(gspGpuHandle)))return ret;

	return cmdbuf[1];
}


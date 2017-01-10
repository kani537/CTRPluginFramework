#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "ctrulib/services/fs.h"
#include "ctrulib/gfx.h"
#include "ctrulib/svc.h"
#include "ctrulib/gpu/gx.h"
#include "ctrulib/os.h"
#define REG(x)   		*(volatile u32*)(x)

#define TOP_SIZE 0x46500
#define BOTTOM_SIZE 0x38400

//GSPGPU_FramebufferInfo topFramebufferInfo;
//GSPGPU_FramebufferInfo bottomFramebufferInfo;
//u8 gfxThreadID;
//u8* gfxSharedMemory;

typedef struct 	s_LCDFamebufferSetup
{
	u16 	width;
	u16 	height;
	u32 	leftScreen1;
	u32 	leftScreen2;
	u32 	format;
	u32 	index;
	u32 	disco;
	u32 	stride;
	u32 	rightScreen1;
	u32 	rightScreen2;
}				t_LCDFramebufferSetup;

static t_LCDFramebufferSetup	*g_topScreen = (void *)0;
static t_LCDFramebufferSetup 	*g_bottomScreen = (void *)0;

static bool enable3d;

//------------------------------------------
//Plugin relative
//------------------------------------------

static bool alreadyAcquired = false;


//-------------------------------------------
/*
void gfxFillColor(gfxScreen_t screen, u32 fillcolor)
{
	u32 i;

	IoBaseLcd = plgGetIoBase(IO_BASE_LCD);
	for (i = 0; i < 0x64; ++i )
	{
		if (screen == GFX_BOTH || screen == GFX_TOP)
			*(u32 *)(IoBaseLcd + 0x204) = fillcolor;
		if (screen == GFX_BOTH || screen == GFX_BOTTOM)
			*(u32 *)(IoBaseLcd + 0xA04) = fillcolor;
		svc_sleepThread(5000000); // 0.005 second
	}
	*(u32*)(IoBaseLcd + 0x204) = 0;
	*(u32*)(IoBaseLcd + 0xA04) = 0;
}


static u32 __get_bytes_per_pixel(GSPGPU_FramebufferFormats format) {
    switch(format) {
    case GSP_RGBA8_OES:
        return 4;
    case GSP_BGR8_OES:
        return 3;
    case GSP_RGB565_OES:
    case GSP_RGB5_A1_OES:
    case GSP_RGBA4_OES:
        return 2;
    }

    return 3;
}

void logGpuReg(void)
{
	new_log(DEBUG, \
	"Framebuffer:\n" \
	"TOP:\n" \
	"left (0): %08X\n" \
	"left (1): %08X\n" \
	"right (0): %08X\n" \
	"right (1): %08X\n" \
	"current: %d\n" \
	"BOTTOM:\n" \
	"(0): %08X\n" \
	"(1): %08X\n" \
	"current: %d\n", \
	REG(IoBasePdc + 0x468),
	REG(IoBasePdc + 0x46C),
	REG(IoBasePdc + 0x494),
	REG(IoBasePdc + 0x498),
	REG(IoBasePdc + 0x478) & 0x1,
	REG(IoBasePdc + 0x568),
	REG(IoBasePdc + 0x56C),
	REG(IoBasePdc + 0x578) & 0x1);
}

static void (*screenFree)(void *) = NULL;

void gfxSaveVRAM(Handle file)
{
	u32 bytesRead;
	u32 result;
	
	new_log(INFO, "gfx Write VRAM to file");
	//result = FSFILE_Write(backFile, &bytesRead, 0, (void *)(gfxBottomFramebuffers[0] + PtoV), 0xFD200, 1);
	//Bottom buffer 0
	memcpy((void *)screenBuffer, gfxBottomFramebuffers[0] + PtoV, BOTTOM_SIZE);
	result = FSFILE_Write(file, &bytesRead, 0, (void *)screenBuffer, BOTTOM_SIZE, 1);
	if (R_FAILED(result)) new_log(WARNING, "Write BOTTOM VRAM to file: FAILED -> %08X", result);
	//bottom buffer 1
	memcpy((void *)screenBuffer, gfxBottomFramebuffers[1] + PtoV, BOTTOM_SIZE);
	result = FSFILE_Write(file, &bytesRead, BOTTOM_SIZE, (void *)screenBuffer, BOTTOM_SIZE, 1);
	if (R_FAILED(result)) new_log(WARNING, "Write BOTTOM2 VRAM to file: FAILED -> %08X", result);
	//top buffer 0
	memcpy((void *)screenBuffer, gfxTopLeftFramebuffers[0] + PtoV, TOP_SIZE);
	result = FSFILE_Write(file, &bytesRead, BOTTOM_SIZE * 2, (void *)screenBuffer, TOP_SIZE, 1);
	if (R_FAILED(result)) new_log(WARNING, "Write TOP VRAM to file: FAILED -> %08X", result);
	//top buffer 0
	memcpy((void *)screenBuffer, gfxTopLeftFramebuffers[1] + PtoV, TOP_SIZE);
	result = FSFILE_Write(file, &bytesRead, (BOTTOM_SIZE * 2) + TOP_SIZE, (void *)screenBuffer, TOP_SIZE, 1);
	if (R_FAILED(result)) new_log(WARNING, "Write TOP2 VRAM to file: FAILED -> %08X", result);
}

void gfxLoadVRAM(Handle file)
{
	u32 bytesRead;
	u32 result;
	
	new_log(INFO, "gfx read VRAM from file");
	//bottom0
	result = FSFILE_Read(file, &bytesRead, 0, (void *)screenBuffer, BOTTOM_SIZE);
	memcpy(gfxBottomFramebuffers[0] + PtoV, (void *)screenBuffer, BOTTOM_SIZE);
	if (R_FAILED(result)) new_log(WARNING, "Read file BOTTOM to VRAM -> %08X", result);
	//bottom1
	result = FSFILE_Read(file, &bytesRead, BOTTOM_SIZE, (void *)screenBuffer, BOTTOM_SIZE);
	memcpy(gfxBottomFramebuffers[1] + PtoV, (void *)screenBuffer, BOTTOM_SIZE);
	if (R_FAILED(result)) new_log(WARNING, "Read file BOTTOM1 to VRAM -> %08X", result);
	//top0
	result = FSFILE_Read(file, &bytesRead, (BOTTOM_SIZE * 2), (void *)screenBuffer, TOP_SIZE);
	memcpy(gfxTopLeftFramebuffers[0] + PtoV, (void *)screenBuffer, TOP_SIZE);
	if (R_FAILED(result)) new_log(WARNING, "Read file TOP to VRAM -> %08X", result);
	//top1
	result = FSFILE_Read(file, &bytesRead, (BOTTOM_SIZE * 2) + TOP_SIZE, (void *)screenBuffer, TOP_SIZE);
	memcpy(gfxTopLeftFramebuffers[1] + PtoV, (void *)screenBuffer, TOP_SIZE);
	if (R_FAILED(result)) new_log(WARNING, "Read file TOP1 to VRAM -> %08X", result);
}

void gfxSaveGameConfig(void)
{
	u32 buffer;
	u32 bytesRead;
	u32 result;

	bakBottomFramebuffers[0]= (u8 *)(REG(IoBasePdc + 0x568));
	bakBottomFramebuffers[1]= (u8 *)(REG(IoBasePdc + 0x56C));
	bakTopLeftFramebuffers[0]= (u8 *)(REG(IoBasePdc + 0x468));
	bakTopLeftFramebuffers[1]= (u8 *)(REG(IoBasePdc + 0x46C));	
	bakTopRightFramebuffers[0]= (u8	*)(REG(IoBasePdc + 0x494));
	bakTopRightFramebuffers[1]= (u8 *)(REG(IoBasePdc + 0x498));	
	buffer = REG(IoBasePdc + 0x478);
	bakCurrentBuffer[0]= buffer & 0x1;
	buffer = REG(IoBasePdc + 0x578);
	bakCurrentBuffer[1]= buffer & 0x1;
	bakScreenFormat[0] = REG(IoBasePdc + 0x470);
	bakScreenFormat[1] = REG(IoBasePdc + 0x570);
	gfxSaveVRAM(backGameVramFile);
}

void gfxRestoreGameConfig(void)
{
	u32 result;
	u32 bytesRead;
	
	//gfxSaveVRAM(backPluginVramFile);
	gfxLoadVRAM(backGameVramFile);
	WRITEU32(IoBasePdc + 0x568, (u32)bakBottomFramebuffers[0]);
	WRITEU32(IoBasePdc + 0x56C, (u32)bakBottomFramebuffers[1]);
	WRITEU32(IoBasePdc + 0x468, (u32)bakTopLeftFramebuffers[0]);
	WRITEU32(IoBasePdc + 0x46C, (u32)bakTopLeftFramebuffers[1]);
	WRITEU32(IoBasePdc + 0x494, (u32)bakTopRightFramebuffers[0]);
	WRITEU32(IoBasePdc + 0x498, (u32)bakTopRightFramebuffers[1]);
	WRITEU8(IoBasePdc + 0x478, bakCurrentBuffer[0]);
	WRITEU8(IoBasePdc + 0x578, bakCurrentBuffer[1]);
	WRITEU32(IoBasePdc + 0x470, bakScreenFormat[0]);
	WRITEU32(IoBasePdc + 0x570, bakScreenFormat[1]);
	
}

void gfxAcquire(void)
{
	u32 stride;
	u32 topWH;
	u32 botWH;
	u32 topFormatValue;
	u32 botFormatValue;
	//static u32 	firstInit = 1;
	
	if (!alreadyAcquired)
	{
		gfxSaveGameConfig();
		alreadyAcquired = true;
	//	if (firstInit) firstInit = 0;
	//	else gfxLoadVRAM(backPluginVramFile);
	}
	stride = 720; // 240 * 3
	topWH = (u32)(240 << 16 | 400);
	botWH = (u32)(240 << 16 | 320);
	topFormatValue = ((1 << 8)|((1 ^ 1) << 6)|((1) << 5)| topFormat);
	botFormatValue = botFormat;	
	//Set our plugin's framebuffers
	WRITEU32(IoBasePdc + 0x45C, topWH); //TopWH
	WRITEU32(IoBasePdc + 0x468, (u32)gfxTopLeftFramebuffers[0]); //TOP Left 0
	WRITEU32(IoBasePdc + 0x46C, (u32)gfxTopLeftFramebuffers[1]); //TOP Left 1
	WRITEU32(IoBasePdc + 0x470, topFormatValue); //TopFormat
	WRITEU8(IoBasePdc + 0x478, !currentBuffer[0]); //Set buffer 0
	WRITEU32(IoBasePdc + 0x490, stride);
	WRITEU32(IoBasePdc + 0x494, (u32)gfxTopRightFramebuffers[0]); //TOP Right 0
	WRITEU32(IoBasePdc + 0x498, (u32)gfxTopRightFramebuffers[1]); //TOP Left 1
	WRITEU32(IoBasePdc + 0x55C, botWH); //BotWH
	WRITEU32(IoBasePdc + 0x568, (u32)gfxBottomFramebuffers[0]); //BOTTOM 0
	WRITEU32(IoBasePdc + 0x56C, (u32)gfxBottomFramebuffers[1]); //BOTTOM 1
	WRITEU32(IoBasePdc + 0x570, botFormatValue); //BotFormat	
	WRITEU8(IoBasePdc + 0x578, !currentBuffer[1]); //Set buffer 0
	WRITEU32(IoBasePdc + 0x590, stride);
	//gfxFillColor(GFX_TOP, 0x100FF00);
}

void gfxRelease(void)
{
	if (alreadyAcquired == false) return;
	//fadeOut();
	executeCommand(GFX_RELEASE);
	//gfxRestoreGameConfig();	
	alreadyAcquired = false;
}

void gfxInit(GSPGPU_FramebufferFormats topFormat, GSPGPU_FramebufferFormats bottomFormat, bool vrambuffers)
{
	u32 bufferPhysicalAddress = 0x18000000;
	u32 ret = 0;	
	gspInit();
	
	gfxSetupBuffer();
	//gfxSaveGameConfig();

	//VA: 0x1F000000 -> PA: 0x18000000
	gfxBottomFramebuffers[0] = (u8 *)bufferPhysicalAddress;
	bufferPhysicalAddress += BOTTOM_SIZE;
	gfxBottomFramebuffers[1] = (u8 *)bufferPhysicalAddress;
	bufferPhysicalAddress += BOTTOM_SIZE;
	gfxTopLeftFramebuffers[0] = (u8 *)bufferPhysicalAddress;
	gfxTopRightFramebuffers[0] = (u8 *)bufferPhysicalAddress;
	bufferPhysicalAddress += TOP_SIZE;
	gfxTopLeftFramebuffers[1] = (u8 *)bufferPhysicalAddress;
	gfxTopRightFramebuffers[1] = (u8 *)bufferPhysicalAddress;
	
	enable3d = false;
	currentBuffer[0] = 0;
	currentBuffer[1] = 0;
	//logGpuReg();
}

void gfxInitDefault(void)
{
	gfxInit(GSP_BGR8_OES, GSP_BGR8_OES, false);
}

u8 *gfxGetFramebuffer(gfxScreen_t screen)
{
	if(screen == GFX_TOP)
	{
		if (currentBuffer[0] == 0) return ((u8 *)gfxTopLeftFramebuffers[0] + PtoV);
		else return ((u8 *)gfxTopLeftFramebuffers[1] + PtoV);
	}
	else
	{
		if (currentBuffer[1] == 0) return ((u8 *)gfxBottomFramebuffers[0] + PtoV);
		else return ((u8 *)gfxBottomFramebuffers[1] + PtoV);
	}
}

void gfxTransferToScreen(gfxScreen_t screen)
{
	if (alreadyAcquired == false) return;
	/*if (screen == GFX_TOP)
		memcpy(gfxTopLeftFramebuffers[0] + PtoV, (void *)screenBuffer, TOP_SIZE);
	if (screen == GFX_BOTTOM)
		memcpy(gfxBottomFramebuffers[0] + PtoV, (void *)screenBuffer, BOTTOM_SIZE);*/
	//new_log(INFO, "GFX: Transfered to screen");
/*	if (screen == GFX_TOP)
	{
		WRITEU8(IoBasePdc + 0x478, currentBuffer[0]);
		currentBuffer[0] = !currentBuffer[0];
	}
	else
	{
		WRITEU8(IoBasePdc + 0x578, currentBuffer[1]);
		currentBuffer[1] = !currentBuffer[1];
	}
}

void gfxTransferFromScreen(gfxScreen_t screen)
{
	if (alreadyAcquired == false) return;
	if (screen == GFX_TOP)
	{
		WRITEU8(IoBasePdc + 0x478, currentBuffer[0]);
		currentBuffer[0] = !currentBuffer[0];
	}
	else
	{
		WRITEU8(IoBasePdc + 0x578, currentBuffer[1]);
		currentBuffer[1] = !currentBuffer[1];
	}
		//memcpy((void *)screenBuffer, gfxTopLeftFramebuffers[0] + PtoV, TOP_SIZE);
	//if (screen == GFX_BOTTOM)
		//memcpy((void *)screenBuffer, gfxBottomFramebuffers[0] + PtoV, BOTTOM_SIZE);
	//new_log(INFO, "GFX: Transfered from screen");
}

void gfxFlushBuffers(gfxScreen_t screen)
{
	if (alreadyAcquired == false) return;
	if (screen == GFX_TOP)
		GSPGPU_FlushDataCache(gfxTopLeftFramebuffers[currentBuffer[0]], TOP_SIZE);
	if (screen == GFX_BOTTOM)
		GSPGPU_FlushDataCache(gfxTopLeftFramebuffers[currentBuffer[1]], BOTTOM_SIZE);
	//new_log(INFO, "GFX: Flushed");
}

static void DrawFadeScreen(gfxScreen_t screen)
{
	uint32_t 	*screen32 = (u32 *)gfxGetFramebuffer(screen);
	u32			screenSize = (screen == GFX_TOP) ? 0x46500 : 0x38400;
	
	//gfxTransferFromScreen(screen);
	for (int i = 0; i++ < screenSize / sizeof(uint32_t); screen32++)
	{
		*screen32 = (*screen32 >> 1) & 0x7F7F7F7F;
		*screen32 += (*screen32 >> 3) & 0x1F1F1F1F; 
		*screen32 += (*screen32 >> 1) & 0x7F7F7F7F; 
	}
}

void fadeOut(void)
{
	for (int i = 32; i--;)
	{ 
		executeCommand(GFX_ACQUIRE);
		DrawFadeScreen(GFX_BOTTOM);
		//executeCommand(GFX_TRANSFER_SCREEN_BOTTOM);
		executeCommand(GFX_FLUSH_BUFFER_BOTTOM);
		DrawFadeScreen(GFX_TOP);		
		//executeCommand(GFX_TRANSFER_SCREEN_TOP);
		executeCommand(GFX_FLUSH_BUFFER_TOP);
		
		//DrawFadeScreen(&top2Screen); 
		
	} 
}*/

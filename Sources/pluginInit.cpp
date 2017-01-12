#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include "NTR.h"
#include <stdlib.h>

extern "C" void abort(void);


u32     fsUserHandle;
u32     sdmcArchive;

extern "C" void    initSystem();

int    main(void);

void abort(void)
{
  for (;;) { }
}

static  u32 threadStack[0x4000];
Handle threadHandle;

namespace CTRPluginFramework
{
    u32     copyRemoteMemory(Handle hDst, u32 ptrDst, Handle hSrc, u32 ptrSrc, u32 size)
    {
        u32     dmaConfig[20] = { 0 };
        u32     hdma = 0;
        u32     state;
        u32     finishState = 0;
        u32     i;
        u32     result;
        bool    firstError = true;

        if ((result = svcFlushProcessDataCache(hSrc, (void *)ptrSrc, size)) != 0)
            goto error;
        if ((result = svcFlushProcessDataCache(hDst, (void *)ptrDst, size)) != 0)
            goto error;
    again:
        if ((result = svcStartInterProcessDma(&hdma, hDst, (void *)ptrDst, hSrc, (void *)ptrSrc, size, dmaConfig)) != 0)
            goto error;
        state = 0;
        if (finishState == 0)
        {
            while (1)
            {
                svcGetDmaState(&state, hdma);
                svcSleepThread(10000);
                result = state;
                svcGetDmaState(&state, hdma);
                if (result == state)
                    break;
            }
            finishState = state;
        }
        else
        {
            for (i = 0; i < 1000000; i++)
            {           
                result = svcGetDmaState(&state, hdma);
                if (state == finishState)
                    break;
                svcSleepThread(10000);
            }
            if (i >= 1000000)
            {
                svcCloseHandle(hdma);
                goto error;
            }
        }
        svcCloseHandle(hdma);
        if ((result = svcInvalidateProcessDataCache(hDst, (void *)ptrDst, size)) != 0)
            goto error;
        return (0);
    error:
        return (-1);
    }

    u32     protectRemoteMemory(Handle hProcess, u32 addr, u32 size)
    {
        return ((u32)svcControlProcessMemory(hProcess, addr, addr, size, 6, 7));
    }

    u32     patchRemoteProcess(u32 pid, u32 addr, u8 *buf, u32 len)
    {
        u32     hProcess;
        s32     ret;

        if (R_FAILED(ret = svcOpenProcess(&hProcess, pid))) goto error;
        if (R_FAILED(ret = protectRemoteMemory(hProcess, ((addr / 0x1000) * 0x1000), 0x1000))) goto error;
        if (R_FAILED(ret = copyRemoteMemory(hProcess, addr, 0xffff8001, (u32)buf, len))) goto error;
        if (hProcess)
            svcCloseHandle(hProcess);
        return (ret);
    error:
        return (-1);
    }

    void    PatchWireless(void)
    {
        u64 tid = Process::GetTitleID();

        // Zelda OOT
      /*  if (tid == 0x0004000000033500 
            || tid == 0x0004000000033600
            || tid == 0x0004000000033400)
        {
            //Handle  proc = 0;
            u16     patch = 0x4770;
            patchRemoteProcess(0x1A, 0x0105AE4, (u8 *)&patch, 2);
        }*/
        if (tid == 0x0004000000175E00 
            || tid == 0x0004000000164800)
        {
            //Sleep(Seconds(5));
            u32     patch  = 0xE3A01000;
            patchRemoteProcess(Process::GetProcessID(), 0x003DFFD0, (u8 *)&patch, 4);
            //*(vu32 *)(0x003DFFD0) = 0xE3A01000;
        }
    }

    void    Initialize(void)
    {
        // Init heap and services   
        initSystem();
        System::Initialize();
        Process::Initialize(true);
        Screen::Initialize();
        // Patch games which deconnects the wireless
        PatchWireless();        
    }
}

using namespace CTRPluginFramework;

void  ThreadInit(u32 arg)
{

    CTRPluginFramework::Initialize();
    // Reduce Priority
    svcSetThreadPriority(threadHandle, 0x3F);
    // Wait for the game to be launched
    Sleep(Seconds(5));

    PageInfo pinfo;
    MemInfo minfo;

    // Protect VRAM
    svcQueryProcessMemory(&minfo, &pinfo, Process::GetHandle(), 0x1F000000);
    svcControlProcessMemory(Process::GetHandle(), minfo.base_addr, minfo.base_addr, minfo.size, 6, 7);

    int ret = main();

    // Free heap and services and exit thread
    exit(ret);
}
extern "C" int   LaunchMainThread(int arg);

int   LaunchMainThread(int arg)
{
    rtReleaseLock(&((NS_CONFIG*)(NS_CONFIGURE_ADDR))->debugBufferLock);
     
    svcCreateThread(&threadHandle, ThreadInit, 0, &threadStack[0x4000], 0x18, -2);
    return (0);
}

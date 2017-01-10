#include "types.h"
#include "ctrulib/svc.h"
#include "CTRPluginFramework.hpp"
#include "libntrplg/ns/ns.h"
#include "libntrplg/sharedfunc.h"
#include "ctrulib/srv.h"

extern "C" int main(void);
extern "C" void abort(void);
u32     fsUserHandle;
u32     sdmcArchive;

void(*__system_retAddr)(void);

void abort(void)
{
  for (;;) { }
}

static  u32 threadStack[0x1000];
Handle hThread;
void    test(u32 i)
{
   // void    Check(void);
    
    
    
    svcSleepThread(0x200000000);
    srvInit();
    rtReleaseLock(&((NS_CONFIG*)(NS_CONFIGURE_ADDR))->debugBufferLock);
    CTRPluginFramework::System::Initialize();
    srvExit();
    svcExitThread();
    //void    Check(void);
}

//namespace CTRPluginFramework
//{
int   main(void)
{
    //initSharedFunc();
    //INIT_SHARED_FUNC(plgGetIoBase, 8);
    rtReleaseLock(&((NS_CONFIG*)(NS_CONFIGURE_ADDR))->debugBufferLock);
    svcCreateThread(&hThread, test, 0, &threadStack[0x1000], 0x3F, -2);
    return (0);
}
//}
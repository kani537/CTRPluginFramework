#include "CTRPluginFramework.hpp"
#include "Hook.hpp"
#include "3DS.h"

extern "C" vu32* hidSharedMem;
extern "C" void aptHookHome(void);
extern "C" u32     g_isHomeBtnPressed;
u32     g_isHomeBtnPressed = 0;

namespace CTRPluginFramework
{
    // 0x00103DC4 then return to 0x00103F38
    // This function is called on the plugin starts, before main
    void    PatchProcess(void)
    {
        Hook    hook;

        hook.Initialize(0x00103DC4, (u32)aptHookHome);
        hook.Enable();
        ///*(u32 *)0x103AA0 = 0xEA000006;
        *(u32 *)0x00103AA4 = 0xE3500006;
        *(u32 *)0x00103AA8 = 0xEA000004;
    }
    static u32 aptParameters[0x1000/4];
    static inline int countPrmWords(u32 hdr)
    {
        return 1 + (hdr&0x3F) + ((hdr>>6)&0x3F);
    }
    static Result aptGetServiceHandle2(Handle *aptuHandle)
    {
            return srvGetServiceHandleDirect(aptuHandle, "APT:U");
    }

    static Result aptSendCommand2(u32* aptcmdbuf)
    {
        Handle aptuHandle;
        Handle aptLockHandle = *reinterpret_cast<Handle *>(0x006687F0);

        if (aptLockHandle) svcWaitSynchronization(aptLockHandle, U64_MAX);

        Result res = aptGetServiceHandle2(&aptuHandle);
        if (R_SUCCEEDED(res))
        {
            u32* cmdbuf = getThreadCommandBuffer();
            memcpy(cmdbuf, aptcmdbuf, 4*countPrmWords(aptcmdbuf[0]));
            res = svcSendSyncRequest(aptuHandle);
            if (R_SUCCEEDED(res))
            {
                memcpy(aptcmdbuf, cmdbuf, 4*16);
                res = aptcmdbuf[1];
            }
            svcCloseHandle(aptuHandle);
        }
        if (aptLockHandle) svcReleaseMutex(aptLockHandle);
        return res;
    }

    static Result APT_GlanceParameter2(NS_APPID appID, void* buffer, size_t bufferSize, NS_APPID* sender, APT_Command* command, size_t* actualSize, Handle* parameter)
    {
        u32 cmdbuf[16];
        cmdbuf[0]=IPC_MakeHeader(0xE,2,0); // 0xE0080
        cmdbuf[1]=appID;
        cmdbuf[2]=bufferSize;

        u32 saved_threadstorage[2];
        u32* staticbufs = getThreadStaticBuffers();
        saved_threadstorage[0]=staticbufs[0];
        saved_threadstorage[1]=staticbufs[1];
        staticbufs[0]=IPC_Desc_StaticBuffer(cmdbuf[2],0);
        staticbufs[1]=(u32)buffer;

        Result ret = aptSendCommand2(cmdbuf);
        staticbufs[0]=saved_threadstorage[0];
        staticbufs[1]=saved_threadstorage[1];

        if (R_SUCCEEDED(ret))
        {
            if (sender)     *sender    = (NS_APPID)cmdbuf[2];
            if (command)    *command   =(APT_Command)cmdbuf[3];
            if (actualSize) *actualSize=cmdbuf[4];
            if (parameter)  *parameter =cmdbuf[6];
            else if (cmdbuf[6]) svcCloseHandle(cmdbuf[6]);
        }

        return ret;
    }

    static Result APT_CancelParameter2(NS_APPID source, NS_APPID dest, bool* success)
    {
        u32 cmdbuf[16];

        cmdbuf[0] = IPC_MakeHeader(0xF,4,0); // 0xF0100
        cmdbuf[1] = source != APPID_NONE;
        cmdbuf[2] = source;
        cmdbuf[3] = dest != APPID_NONE;
        cmdbuf[4] = dest;

        Result ret = aptSendCommand2(cmdbuf);
        if (R_SUCCEEDED(ret) && success)
            *success = cmdbuf[2] & 0xFF;

        return ret;
    }
    void    CleanAPT(void)
    {
        for (;;)
        {
            APT_Command cmd;
            Result res = APT_GlanceParameter((NS_APPID)envGetAptAppId(), aptParameters, sizeof(aptParameters), NULL, &cmd, NULL, NULL);
            if (R_FAILED(res) || cmd==APTCMD_NONE){ OSD::Notify("Failed"); break;}
            OSD::Notify("Success");
            APT_CancelParameter(APPID_NONE, (NS_APPID)envGetAptAppId(), NULL);
        }
        g_isHomeBtnPressed = 0;
    }

    MenuEntry *AddArg(void *arg, MenuEntry *entry)
    {
        if(entry != nullptr)
            entry->SetArg(arg);
        return (entry);
    }

    MenuEntry *EntryWithHotkey(MenuEntry *entry, const Hotkey &hotkey)
    {
        if (entry != nullptr)
        {
            entry->Hotkeys += hotkey;
            entry->SetArg(new std::string(entry->Name()));
            entry->Name() += " " + hotkey.ToString();
            entry->Hotkeys.OnHotkeyChangeCallback([](MenuEntry *entry, int index)
            {
                std::string *name = reinterpret_cast<std::string *>(entry->GetArg());

                entry->Name() = *name + " " + entry->Hotkeys[0].ToString();
            });
        }            

        return (entry);
    }

    MenuEntry *EntryWithHotkey(MenuEntry *entry, const std::vector<Hotkey> &hotkeys)
    {
        if (entry != nullptr)
        {
            for (const Hotkey &hotkey : hotkeys)
                entry->Hotkeys += hotkey;
        }

        return (entry);
    }

#define BLANKVERSION 1

#if BLANKVERSION
    int     main(void)
    {
        Process::ProtectRegion((u32)hidSharedMem, MEMPERM_READ | MEMPERM_WRITE);
        Directory::ChangeWorkingDirectory("/3ds/ntr/plugin/" + Utils::Format("%016llX/", Process::GetTitleID()));
        static const char *about = "This is a blank plugin to let you use ctrpf on multiple games without being annoyed by builtin cheats.";
        PluginMenu  *m = new PluginMenu("Blank plugin", 0, 3, 0, about);
        PluginMenu  &menu = *m;

        menu += []
        {
            if (g_isHomeBtnPressed)
            {
                OSD::Notify("Home btn pressed");
                CleanAPT();
            }
        };

#else
    extern MenuFolder  *g_folder;

    void    LineReadTest(MenuEntry *entry);
    int     main(void)
    {
        Directory::ChangeWorkingDirectory("/plugin/game/");
        Sleep(Seconds(5.f));
        PluginMenu  *m = new PluginMenu("Action Replay Test", 0, 0, 1);
        PluginMenu  &menu = *m;

        menu += new MenuEntry("Load cheats from file", nullptr, LineReadTest);
        menu += g_folder;
#endif
        menu += []
        {
            Sleep(Milliseconds(5));
        };

        // Launch menu and mainloop
        menu.Run();

        // Exit plugin
        return (0);
    }
}

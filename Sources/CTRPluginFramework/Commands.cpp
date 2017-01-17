#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include "ctrulib/gpu/gx.h"
#include "ctrulib/gpu/gpu.h"
#include <cstdio>

namespace CTRPluginFramework
{
    char     ThreadCommands::_stack[0x1000] = {0};
    Handle   ThreadCommands::_threadHandle = 0;
    Handle   ThreadCommands::_threadEvent = 0;
    Commands ThreadCommands::_command = Commands::NONE;
    int      ThreadCommands::_arg1 = 0;
    int      ThreadCommands::_arg2 = 0;

    volatile bool     ThreadCommands::_isBusy = 0;

    int     ReadFile(int arg1)
    {
        FileCommand *fc = (FileCommand *)arg1;
        if (!fc)
        {
            //Color r = Color(255, 0, 0);
            //Screen::Bottom->Flash(r);
            return (-1);
        }
        FILE    *file = fc->file;
        u8      *dst = fc->dst;
        u32     size = fc->size;
        u32     br = 0;

        br = std::fread(dst, 1, size, file);
        fc->read = br;
        return (br);
    }

    void    ThreadCommands::ThreadCommandsMain(void *arg)
    {
        static bool isRunning = true;

        while (isRunning)
        {
            svcWaitSynchronization(_threadEvent, U64_MAX);
            svcClearEvent(_threadEvent);
            _isBusy = true;
            switch (_command)
            {
                case Commands::NONE:
                    break;
                case Commands::GSPGPU_END:
                    /*
                    ** Bottom size
                    ** Bottom FB
                    ** Top size
                    ** IS3D
                    ** Top LFB
                    ** Top RFB
                    **/
                    u32     infos[6];
                    Renderer::GetFramebuffersInfos(infos);
                    GSPGPU_FlushDataCache((void *)infos[4], infos[2]);
                    if (infos[3])
                        GSPGPU_FlushDataCache((void *)infos[5], infos[2]);
                    GSPGPU_FlushDataCache((void *)infos[1], infos[0]);
                    Screen::Top->SwapBuffer();
                    Screen::Bottom->SwapBuffer();
                    gspWaitForVBlank();
                    break;
                case Commands::GSPGPU_FLUSH:
                    GSPGPU_FlushDataCache((void *)_arg1, _arg2);
                    break;
                case Commands::GSPGPU_INVALIDATE:
                    GSPGPU_InvalidateDataCache((void *)_arg1, _arg2);
                    break;
                case Commands::GSPGPU_SWAP:
                    gfxSwapBuffers();
                    break;
                case Commands::GSPGPU_VBLANK:                      
                    gspWaitForVBlank();
                    break;
                case Commands::GSPGPU_VBLANK1:                      
                    gspWaitForVBlank1();
                    break;
                case Commands::FS_READFILE:
                    ReadFile(_arg1);
                    break;
                case Commands::EXIT:
                    isRunning = false;
                    break;
            }
            _isBusy = false;
            _arg1 = 0;
            _arg2 = 0;
        }
        _isBusy = false;        
        svcExitThread();
    }

    void    ThreadCommands::Initialize(void)
    {
        svcCreateEvent(&_threadEvent, RESET_ONESHOT);
        Thread t = threadCreate(ThreadCommandsMain, _stack, 0x1000, 0x18, -2, false);
        _threadHandle = t->handle;
        //svcCreateThread(&_threadHandle, ThreadCommandsMain, 0, (u32 *)&_stack[0x1000], 0x18, -2);
    }

    void    ThreadCommands::Pause(void)
    {

        svcSetThreadPriority(_threadHandle, 0x3F);
    }

    void    ThreadCommands::Play(void)
    {

        svcSetThreadPriority(_threadHandle, 0x18);
    }

    void    ThreadCommands::Exit(void)
    {
        Execute(Commands::EXIT);
        svcWaitSynchronization(_threadHandle, U64_MAX);
    }

    void    ThreadCommands::SetArgs(int arg1, int arg2)
    {
        _arg1 = arg1;
        _arg2 = arg2;
    }

    void    ThreadCommands::Execute(Commands command, bool isBlocking)
    {
        _command = command;
        svcSignalEvent(_threadEvent);
        if (isBlocking)
        {
            while (_isBusy)
                for (int i = 1000; i > 0; i--);
        }
    }
}
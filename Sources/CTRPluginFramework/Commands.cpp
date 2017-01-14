#include "CTRPluginFramework.hpp"
#include "3DS.h"
#include "ctrulib/gpu/gx.h"
#include "ctrulib/gpu/gpu.h"

namespace CTRPluginFramework
{
    char     ThreadCommands::_stack[0x1000] = {0};
    Handle   ThreadCommands::_threadHandle = 0;
    Handle   ThreadCommands::_threadEvent = 0;
    Commands ThreadCommands::_command = Commands::NONE;
    int      ThreadCommands::_arg1 = 0;
    int      ThreadCommands::_arg2 = 0;

    volatile bool     ThreadCommands::_isBusy = 0;

    void    ThreadCommands::ThreadCommandsMain(u32 arg)
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
        svcCreateThread(&_threadHandle, ThreadCommandsMain, 0, (u32 *)&_stack[0x1000], 0x18, -2);
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
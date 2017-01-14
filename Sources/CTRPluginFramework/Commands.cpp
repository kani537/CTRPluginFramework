#include "CTRPluginFramework.hpp"
#include "3DS.h"

namespace CTRPluginFramework
{
    char     ThreadCommands::_stack[0x1000] = {0};
    Handle   ThreadCommands::_threadHandle = 0;
    Handle   ThreadCommands::_threadEvent = 0;
    Commands ThreadCommands::_command = Commands::NONE;
    volatile bool     ThreadCommands::_isBusy = 0;
    int      ThreadCommands::_arg1 = 0;
    int      ThreadCommands::_arg2 = 0;

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
                case Commands::GSPGPU_FLUSH:
                    GSPGPU_FlushDataCache((void *)_arg1, _arg2);
                    break;
                case Commands::GSPGPU_INVALIDATE:
                    GSPGPU_InvalidateDataCache((void *)_arg1, _arg2);
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
            while (_isBusy);
                for (int i = 1000; i > 0; i--);
        }
    }
}
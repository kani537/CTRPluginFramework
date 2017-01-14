#ifndef CTRPLUGINFRAMEWORK_COMMANDS_HPP
#define CTRPLUGINFRAMEWORK_COMMANDS_HPP

#include "types.h"
#include "ctrulib/svc.h"

namespace CTRPluginFramework
{
    enum class Commands
    {
        NONE,
        GSPGPU_FLUSH,
        GSPGPU_INVALIDATE,
        GSPGPU_SWAP,
        GSPGPU_VBLANK,
        GSPGPU_VBLANK1,
        EXIT
    };

    class ThreadCommands
    {
    public:
        static void     SetArgs(int arg1 = 0, int arg2 = 0);
        static void     Execute(Commands command, bool isBlocking = true);
        static void     Exit(void);

    private:
        static void     ThreadCommandsMain(u32 arg);
        friend void     Initialize(void);
        static void     Initialize(void);

        static char     _stack[0x1000];
        static Handle   _threadHandle;
        static Handle   _threadEvent;
        static Commands _command;
        static int      _arg1;
        static int      _arg2;
        static volatile bool     _isBusy;

    };
}

#endif
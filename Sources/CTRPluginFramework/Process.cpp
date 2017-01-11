#include "CTRPluginFramework.hpp"
#include "arm11kCommands.h"

namespace CTRPluginFramework
{
    u32         Process::_processID = 0;
    u64         Process::_titleID = 0;
    char        Process::_processName[8] = {0};
    u32         Process::_kProcess = 0;
    //u32         *Process::_kProcessHandleTable = nullptr;
    KCodeSet    Process::_kCodeSet = {0};


    void    Process::Initialize(bool isNew3DS)
    {
        char    kproc[0x100] = {0};

        // Get current KProcess
        _kProcess = (u32)arm11kGetCurrentKProcess();
        // Copy KProcess data
        arm11kMemcpy((u32)&kproc, _kProcess, 0x100);
        if (isNew3DS)
        {
            // Copy KCodeSet
            arm11kMemcpy((u32)&_kCodeSet, *(u32 *)(kproc + 0xB8), 0x64);          

            // Copy process id
            _processID = *(u32 *)(kproc + 0xBC);
        }
        else
        {
            // Copy KCodeSet
            arm11kMemcpy((u32)&_kCodeSet, *(u32 *)(kproc + 0xB0), 0x64);          

            // Copy process id
            _processID = *(u32 *)(kproc + 0xB4);
        }

        // Copy process name
        for (int i = 0; i < 8; i++)
                _processName[i] = _kCodeSet.processName[i];

        // Copy title id
        _titleID = _kCodeSet.titleId;
    }

    u32     Process::GetID(void)
    {
        return (_processID);
    }

    u64     Process::GetTitleID(void)
    {
        return (_titleID);
    }

    void    Process::GetName(char *output)
    {
        if (output != nullptr)
            for (int i = 0; i < 8; i++)
                output[i] = _processName[i];
    }
}
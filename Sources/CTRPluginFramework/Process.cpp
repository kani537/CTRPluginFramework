#include "CTRPluginFramework.hpp"
#include "arm11kCommands.h"

namespace CTRPluginFramework
{
    u32         Process::_processID = 0;
    u32         *Process::_kProcess = nullptr;
    u32         *Process::_kProcessHandleTable = nullptr;
    KCodeSet     *Process::_kCodeSet = nullptr;

    void    Process::Initialize(bool isNew3DS)
    {
        _kProcess = (u32 *)arm11kGetCurrentKProcess();
        if (isNew3DS)
        {
            _kCodeSet = (KCodeSet *)(_kProcess + 0xB8);
            _processID = *(u32 *)(_kProcess + 0xB4);
            _kProcessHandleTable = (u32 *)(_kProcess + 0xDC);

        }
        else
        {
            _kCodeSet = (KCodeSet *)(_kProcess + 0xB0);            
            _processID = *(u32 *)(_kProcess + 0xBC);
            _kProcessHandleTable = (u32 *)(_kProcess + 0xD4);
        }
    }

    u32     Process::GetID(void)
    {
        return (_processID);
    }

    u64     Process::GetTitleID(void)
    {
        if (_kCodeSet == nullptr)
            return (0);
        return (_kCodeSet->titleId);
    }

    void    Process::GetName(char *output)
    {
        if (!output || _kCodeSet == nullptr)
            return;
        for (int i = 0; i < 8; i++)
            output[i] = _kCodeSet->processName[i];
    }
}
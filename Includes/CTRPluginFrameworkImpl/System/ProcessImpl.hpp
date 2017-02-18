#ifndef CTRPLUGINFRAMEWORKIMPL_PROCESSIMPL_HPP
#define CTRPLUGINFRAMEWORKIMPL_PROCESSIMPL_HPP

#include "ctrulib/svc.h"

namespace CTRPluginFramework
{
    struct KCodeSetMemDescriptor
    {
        u32     startAddress;
        u32     totalPages;
        u32     kBlockInfoCount;
        u32     *firstKLinkedNode;
        u32     *lastKLinkedNode;
    }__attribute__((packed));

    struct KCodeSet
    {
        u32     *vtable;
        u32     refCount;
        KCodeSetMemDescriptor text;
        KCodeSetMemDescriptor rodata;
        KCodeSetMemDescriptor data;
        u32     textPages;
        u32     rodataPages;
        u32     rwPages;
        char    processName[8];
        u32     unknown;
        u64     titleId;
    }__attribute__((packed));

    class ProcessImpl
    {
        public:
            // Pause the current process
            static void     Pause(bool useFading);
            // Unpause the current process
            static void     Play(bool useFading);
            
            static bool     IsPaused(void);
            static bool     IsAcquiring(void);
            static void     Initialize(void);
            static void     UpdateThreadHandle(void);
            static bool     PatchProcess(u32 addr, u8 *patch, u32 length, u8 *original);

           
            static u32          _processID;
            static u64          _titleID;
            static char         _processName[8];
            static u32          _kProcess;
            static u32          _kProcessState;
            static KCodeSet     _kCodeSet;
            static Handle       _processHandle;
            static Handle       _mainThreadHandle;
            static bool         _isPaused;
            static bool         _isAcquiring;
            //static u32          _finishedStateDMA;
            //static u32          *_kProcessHandleTable;          
    };
}

#endif
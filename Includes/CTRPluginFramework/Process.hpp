#ifndef CTRPLUGINFRAMEWORK_PROCESS_HPP
#define CTRPLUGINFRAMEWORK_PROCESS_HPP

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

    class Process
    {
        public:
            // Return current process handle (already open, don't close it)
            static Handle   GetHandle(void);
            // Return current process ID
            static u32      GetProcessID(void);
            // Copy current process ID in output as string
            static void     GetProcessID(char *output);  
            // Return current process title ID          
            static u64      GetTitleID(void);
            // Copy current process title ID in output as string
            static void     GetTitleID(char *output);
            // Copy current process name in output
            static void     GetName(char *output);
            // Return the status of the process (should always return 1)
            static u8       GetProcessState(void);

            // Safely patch the current process
            // The original data in the target address can be obtained by passing a pointer to a buffer
            static bool     Patch(u32 addr, u8 *patch, u32 length, u8 *original = nullptr);
            // Protect the memory by settings Read & write perm
            static bool     ProtectMemory(u32  addr, u32 size, int perm = (MEMPERM_READ | MEMPERM_WRITE |MEMPERM_EXECUTE));
            // Protect the entire region where addr belongs to
            // Return false if the addr doesn't exists or if the protect operation failed
            static bool     ProtectRegion(u32 addr, int perm = (MEMPERM_READ | MEMPERM_WRITE |MEMPERM_EXECUTE));
            // Safely copy the current process memory
            static bool     CopyMemory(void *dst, void *src, u32 size);
            // Pause the current process
            static void     Pause(void);
            // Unpause the current process
            static void     Play(bool isInit = false);
            static bool     IsPaused(void);
            static bool     IsAcquiring(void);

        private:
            friend void     Initialize(void);
            friend void     KeepThreadMain(void *arg);

            static void     Initialize(Handle keepEvent);
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
            static Handle       _keepEvent;
            static bool         _isPaused;
            static bool         _isAcquiring;
            //static u32          _finishedStateDMA;
            //static u32          *_kProcessHandleTable;          
    };
}

#endif
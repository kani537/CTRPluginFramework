#ifndef CTRPLUGINFRAMEWORK_PROCESS_HPP
#define CTRPLUGINFRAMEWORK_PROCESS_HPP

#include "CTRPluginFramework.hpp"

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
            static Handle   GetHandle(void);
            static u32      GetProcessID(void);
            static void     GetProcessID(char *output);            
            static u64      GetTitleID(void);
            static void     GetTitleID(char *output);
            static void     GetName(char *output);
            static u8       GetProcessState(void);
            static void     Pause(void);
            static void     Play(void);

        private:
            friend void Initialize(void);
            static void Initialize(Handle threadHandle, bool isNew3DS);
           
            static u32          _processID;
            static u64          _titleID;
            static char         _processName[8];
            static u32          _kProcess;
            static u32          _kProcessState;
            static KCodeSet     _kCodeSet;
            static Handle       _handle;
            static Handle       _mainThreadHandle;
            //static u32          *_kProcessHandleTable;          
    };
}

#endif
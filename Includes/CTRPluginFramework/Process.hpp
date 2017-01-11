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
    };

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
        u16     unknown;
        u16     unknown1;
        u64     titleId;
    };

    class Process
    {
        public:
            static u32  GetID(void);            
            static u64  GetTitleID(void);
            static void GetName(char *output);

        private:
            friend class System;
            static void Initialize(bool isNew3DS);
           
            static u32          _processID;
            static u32          *_kProcess;
            static u32          *_kProcessHandleTable;          
            static KCodeSet     *_kCodeSet;
    };
}

#endif
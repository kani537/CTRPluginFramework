#ifndef NEWLIBHEAP_H
#define NEWLIBHEAP_H

#include "types.h"

u8      *getHeapStart(void);
u8      *getHeapEnd(void);
u8      *getHeapLimit(void);
int     getMemUsed(void);
int     getMemFree(void);

#endif
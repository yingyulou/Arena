#pragma once

#include "Bitmap.h"
#include "Queue.h"
#include "Util.h"

#define TASK_READY ((uint32_t)0x0)
#define TASK_EXIT  ((uint32_t)0x1)
#define TASK_BLOCK ((uint32_t)0x2)

typedef struct
{
    Node _;
    uint32_t __CR3;
    uint32_t __ESP0;
    uint32_t __taskState;
    Bitmap __vBitmap;
    uint8_t __TSS[104];
} TCB;


extern TCB *curTask;

void taskInit();
TCB *loadTaskPL0(void *EIP);
void loadTaskPL3(uint32_t startSector, uint8_t sectorCount);
TCB *getNextTask();
void taskExit();

#pragma once

#include "Bitmap.h"
#include "Queue.h"
#include "Util.h"

#define TASK_READY ((uint32_t)0x0)
#define TASK_EXIT  ((uint32_t)0x1)
#define TASK_BLOCK ((uint32_t)0x2)

typedef struct
{
    Node tcbNode;
    uint32_t CR3;
    uint32_t ESP0;
    uint32_t taskState;
    Bitmap vMemoryBitmap;
} TCB;


extern uint8_t TSS[];
extern Queue taskQueue;
extern TCB *curTask;

void taskInit();
TCB *loadTaskPL0(uint32_t EIP);
void loadTaskPL3(uint32_t startSector, uint32_t sectorCount);
TCB *getNextTask();
void taskExit();

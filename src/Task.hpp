#pragma once

#include "Task.h"
#include "Bitmap.h"
#include "Queue.h"
#include "Memory.h"
#include "HD.h"
#include "Shell.h"
#include "Util.h"

uint8_t TSS[104];
TCB *curTask = 0;
Queue __taskQueue;

uint64_t __makeTSSDescriptor(uint64_t tssBase, uint64_t tssLimit, uint64_t tssAttr)
{
    return (tssLimit & 0xffff) | ((tssBase & 0xffffff) << 16) | (tssAttr << 32) |
        ((tssLimit & 0xf0000) << 32) | ((tssBase & 0xff000000) << 32);
}


void __tssInit()
{
    *(uint16_t *)(TSS + 8)   = 2 << 3;
    *(uint16_t *)(TSS + 102) = 103;

    uint64_t GDTR;

    __asm__ __volatile__("sgdt %0":: "m"(GDTR));

    ((uint64_t *)(uint32_t)(GDTR >> 16))[5] = __makeTSSDescriptor((uint32_t)TSS, 103, 0x8900);

    __asm__ __volatile__("ltr %w0":: "r"(5 << 3));
}


void __kernelTaskInit()
{
    TCB *tcbPtr = (TCB *)0xc009f000;

    tcbPtr->__CR3       = 0x100000;
    tcbPtr->__taskState = TASK_READY;

    curTask = tcbPtr;
}


void taskInit()
{
    queueInit(&__taskQueue);

    __tssInit();
    __kernelTaskInit();
}


uint32_t __getEFLAGS()
{
    uint32_t EFLAGS;

    __asm__ __volatile__("pushf; pop %0": "=g"(EFLAGS));

    return EFLAGS;
}


TCB *loadTaskPL0(void *EIP)
{
    uint8_t *taskMemPtr = (uint8_t *)allocateKernelPage(3);

    TCB *tcbPtr         = (TCB *)taskMemPtr;
    uint32_t vCR3       = (uint32_t)(taskMemPtr + 0x1000);
    uint32_t pCR3       = *(uint32_t *)(0xffc00000 | (vCR3 >> 12 << 2)) & 0xfffff000;
    uint8_t *vBitmapBuf = taskMemPtr + 0x2000;
    uint32_t *ESP0      = (uint32_t *)((uint32_t)tcbPtr + 0x1000 - 15 * 4);

    tcbPtr->__CR3       = pCR3;
    tcbPtr->__ESP0      = (uint32_t)ESP0;
    tcbPtr->__taskState = TASK_READY;
    bitmapInit(&tcbPtr->__vBitmap, vBitmapBuf, 0x8000);

    memcpy((void *)(vCR3 + 0xc00), (void *)0xfffffc00, 255 * 4);
    ((uint32_t *)vCR3)[1023] = pCR3 | 0x3;

    ESP0[0]  = 0;
    ESP0[1]  = 0;
    ESP0[2]  = 0;
    ESP0[3]  = 0;
    ESP0[4]  = 0;
    ESP0[5]  = 0;
    ESP0[6]  = 0;
    ESP0[7]  = 0;
    ESP0[8]  = 2 << 3;
    ESP0[9]  = 2 << 3;
    ESP0[10] = 2 << 3;
    ESP0[11] = 2 << 3;
    ESP0[12] = (uint32_t)EIP;
    ESP0[13] = 1 << 3;
    ESP0[14] = __getEFLAGS() | 0x200;

    queuePush(&__taskQueue, (Node *)tcbPtr);

    return tcbPtr;
}


void loadTaskPL3(uint32_t startSector, uint8_t sectorCount)
{
    uint8_t *taskMemPtr = (uint8_t *)allocateKernelPage(3);

    TCB *tcbPtr         = (TCB *)taskMemPtr;
    uint32_t vCR3       = (uint32_t)(taskMemPtr + 0x1000);
    uint32_t pCR3       = *(uint32_t *)(0xffc00000 | (vCR3 >> 12 << 2)) & 0xfffff000;
    uint8_t *vBitmapBuf = taskMemPtr + 0x2000;
    uint32_t *ESP0      = (uint32_t *)((uint32_t)tcbPtr + 0x1000 - 17 * 4);
    uint32_t curCR3;

    tcbPtr->__CR3       = pCR3;
    tcbPtr->__ESP0      = (uint32_t)ESP0;
    tcbPtr->__taskState = TASK_READY;
    bitmapInit(&tcbPtr->__vBitmap, vBitmapBuf, 0x8000);

    memcpy((void *)(vCR3 + 0xc00), (void *)0xfffffc00, 255 * 4);
    ((uint32_t *)vCR3)[1023] = pCR3 | 0x3;

    __asm__ __volatile__(
        "mov %%cr3, %0\n\t"
        "mov %1, %%cr3\n\t"
        : "=&r"(curCR3)
        : "r"(pCR3)
    );

    uint8_t *elfBuf = (uint8_t *)0xc0080000;

    hdRead(elfBuf, startSector, sectorCount);

    uint32_t EIP     = *(uint32_t *)(elfBuf + 0x18);
    uint8_t *phPtr   = elfBuf + *(uint32_t *)(elfBuf + 0x1c);
    uint32_t phSize  = *(uint16_t *)(elfBuf + 0x2a);
    uint32_t phCount = *(uint16_t *)(elfBuf + 0x2c);

    for (uint32_t _ = 0; _ < phCount; _++, phPtr += phSize)
    {
        if (*(uint32_t *)phPtr == 0x1)
        {
            void *srcPtr      = elfBuf + *(uint32_t *)(phPtr + 0x4);
            void *tarPtr      = *(void **)(phPtr + 0x8);
            uint32_t fileSize = *(uint32_t *)(phPtr + 0x10);
            uint32_t memSize  = *(uint32_t *)(phPtr + 0x14);

            installTaskPage(&tcbPtr->__vBitmap, tarPtr, memSize);
            memcpy(tarPtr, srcPtr, fileSize);
        }
    }

    ESP0[0]  = 0;
    ESP0[1]  = 0;
    ESP0[2]  = 0;
    ESP0[3]  = 0;
    ESP0[4]  = 0;
    ESP0[5]  = 0;
    ESP0[6]  = 0;
    ESP0[7]  = 0;
    ESP0[8]  = (4 << 3) | 0x3;
    ESP0[9]  = (4 << 3) | 0x3;
    ESP0[10] = (4 << 3) | 0x3;
    ESP0[11] = (4 << 3) | 0x3;
    ESP0[12] = EIP;
    ESP0[13] = (3 << 3) | 0x3;
    ESP0[14] = __getEFLAGS() | 0x200;
    ESP0[15] = 0xc0000000;
    ESP0[16] = (4 << 3) | 0x3;

    installTaskPage(&tcbPtr->__vBitmap, (void *)(0xc0000000 - 0x1000), 0x1000);

    __asm__ __volatile__("mov %0, %%cr3":: "r"(curCR3));

    queuePush(&__taskQueue, (Node *)tcbPtr);
}


TCB *getNextTask()
{
    queuePush(&__taskQueue, (Node *)curTask);

    for (;;)
    {
        TCB *tcbPtr = (TCB *)queuePop(&__taskQueue);

        switch (tcbPtr->__taskState)
        {
            case TASK_READY:
                curTask = tcbPtr;
                return tcbPtr;
                break;

            case TASK_EXIT:
                deallocateKernelPage(tcbPtr, 3);
                break;

            case TASK_BLOCK:
                queuePush(&__taskQueue, (Node *)tcbPtr);
                break;

            default:
                printf("Invalid task state\n");
                __asm__ __volatile__("cli; hlt");
                break;
        }
    }
}


void taskExit()
{
    deallocateTaskCR3();

    curTask->__taskState   = TASK_EXIT;
    shellTask->__taskState = TASK_READY;

    __asm__ __volatile__("jmp __taskSwitch");
}

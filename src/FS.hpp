#pragma once

#include "FS.h"
#include "Print.h"
#include "Task.h"
#include "HD.h"
#include "Util.h"

FCB __fcbList[16];
uint8_t __hdBitmapBuf[512];
Bitmap __hdBitmap;

void fsInit()
{
    hdRead(__fcbList, 100, 1);

    if (*(uint32_t *)(__fcbList + 15) == 0x66666666)
    {
        hdRead(__hdBitmapBuf, 101, 1);

        bitmapInit(&__hdBitmap, __hdBitmapBuf, 512 * 8, false);
    }
    else
    {
        memset(__fcbList, 0, 512);
        *(uint32_t *)(__fcbList + 15) = 0x66666666;

        bitmapInit(&__hdBitmap, __hdBitmapBuf, 512 * 8, true);

        hdWrite(__fcbList, 100, 1);
        hdWrite(__hdBitmapBuf, 101, 1);
    }
}


void fsList()
{
    for (uint32_t idx = 0; idx < 15; idx++)
    {
        if (__fcbList[idx].__startSector)
        {
            printf("%s %d %d\n", __fcbList[idx].__fileName, __fcbList[idx].__startSector, __fcbList[idx].__sectorCount);
        }
    }
}


uint32_t __allocateFCB()
{
    for (uint32_t idx = 0; idx < 15; idx++)
    {
        if (!__fcbList[idx].__startSector)
        {
            return idx;
        }
    }

    return -1;
}


void fsCreate(const char *fileName, uint32_t startSector, uint32_t sectorCount)
{
    uint32_t fcbIdx = __allocateFCB();

    if (fcbIdx == -1)
    {
        return;
    }

    strcpy(__fcbList[fcbIdx].__fileName, fileName, 24);
    __fcbList[fcbIdx].__startSector = bitmapAllocate(&__hdBitmap, sectorCount) + 102;
    __fcbList[fcbIdx].__sectorCount = sectorCount;

    uint8_t hdBuf[512];

    for (uint32_t idx = 0; idx < sectorCount; idx++)
    {
        hdRead(hdBuf, startSector + idx, 1);
        hdWrite(hdBuf, __fcbList[fcbIdx].__startSector + idx, 1);
    }

    hdWrite(__fcbList, 100, 1);
    hdWrite(__hdBitmapBuf, 101, 1);
}


uint32_t __findFCB(const char *fileName)
{
    for (uint32_t idx = 0; idx < 15; idx++)
    {
        if (__fcbList[idx].__startSector && strcmp(__fcbList[idx].__fileName, fileName))
        {
            return idx;
        }
    }

    return -1;
}


void fsDelete(const char *fileName)
{
    uint32_t fcbIdx = __findFCB(fileName);

    if (fcbIdx == -1)
    {
        return;
    }

    bitmapDeallocate(&__hdBitmap, __fcbList[fcbIdx].__startSector - 102, __fcbList[fcbIdx].__sectorCount);
    memset(__fcbList + fcbIdx, 0, 32);

    hdWrite(__fcbList, 100, 1);
    hdWrite(__hdBitmapBuf, 101, 1);
}


void fsLoad(const char *fileName)
{
    uint32_t fcbIdx = __findFCB(fileName);

    if (fcbIdx != -1)
    {
        loadTaskPL3(__fcbList[fcbIdx].__startSector, __fcbList[fcbIdx].__sectorCount);
    }
}

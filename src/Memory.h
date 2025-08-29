#pragma once

#include "Bitmap.h"
#include "Util.h"

void memoryInit();
void *allocateKernelPage(uint32_t pageCount);
void installTaskPage(Bitmap *memBitmap, void *startPtr, uint32_t memSize);
void deallocateKernelPage(void *startPtr, uint32_t pageCount);
void deallocateTaskCR3();

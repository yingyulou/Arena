#pragma once

#include "Util.h"

void memoryInit();
void *allocateKernelPage(uint32_t pageCount);
void installTaskPage(uint32_t vAddr, uint32_t pageCount);
void deallocateKernelPage(void *vAddr, uint32_t pageCount);
void deallocateTaskCR3();

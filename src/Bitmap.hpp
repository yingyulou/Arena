#pragma once

#include "Bitmap.h"
#include "Memory.h"
#include "Util.h"

void bitmapInit(Bitmap *this, uint8_t *data, uint32_t size, bool zeroBool)
{
    this->__data = data;
    this->__size = size;

    if (zeroBool)
    {
        memset(data, 0, size >> 3);
    }
}


bool bitmapGet(Bitmap *this, uint32_t idx)
{
    return (this->__data[idx >> 3] >> (idx & 7)) & 1;
}


void bitmapSet(Bitmap *this, uint32_t idx, bool val)
{
    if (val)
    {
        this->__data[idx >> 3] |= 1 << (idx & 7);
    }
    else
    {
        this->__data[idx >> 3] &= ~(1 << (idx & 7));
    }
}


uint32_t bitmapAllocate(Bitmap *this, uint32_t bitCount)
{
    for (uint32_t i = 0;; i++)
    {
        bool allocateBool = true;

        for (uint32_t j = i; j < i + bitCount; j++)
        {
            if (bitmapGet(this, j))
            {
                allocateBool = false;
                break;
            }
        }

        if (allocateBool)
        {
            for (uint32_t j = i; j < i + bitCount; j++)
            {
                bitmapSet(this, j, 1);
            }

            return i;
        }
    }
}


void bitmapDeallocate(Bitmap *this, uint32_t startIdx, uint32_t bitCount)
{
    for (uint32_t _ = 0; _ < bitCount; _++)
    {
        bitmapSet(this, startIdx++, 0);
    }
}

#pragma once
#include "pch.h"

struct BufferAllocation
{
    u64 Offset = 0;
    u64 Size = 0;

    bool IsValid() const
    {
        return Size != 0;
    }
};
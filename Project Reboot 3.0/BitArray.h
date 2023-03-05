#pragma once

#include "ContainerAllocationPolicies.h"

class TBitArray
{
private:
    template <typename ArrayType>
    friend class TSparseArray;
    template <typename SetType>
    friend class TSet;

private:
    TInlineAllocator<4>::ForElementType<unsigned int> Data;
    int NumBits;
    int MaxBits;
};
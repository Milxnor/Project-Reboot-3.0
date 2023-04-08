#pragma once

#include "inc.h"


/*
typedef TAlignedBytes<16, 16> FAlignedInlineDelegateType;
#if USE_SMALL_DELEGATES
typedef FHeapAllocator FDelegateAllocatorType;
#else
typedef TInlineAllocator<2> FDelegateAllocatorType;
#endif
*/

class FDelegateBase
{
public:
	// FDelegateAllocatorType::ForElementType<FAlignedInlineDelegateType> DelegateAllocator;
	// int32 DelegateSize;
};
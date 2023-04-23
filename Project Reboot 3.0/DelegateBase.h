#pragma once

#include "inc.h"

#include "TypeCompatibleBytes.h"
#include "ContainerAllocationPolicies.h"

#if !defined(_WIN32) || defined(_WIN64)
// Let delegates store up to 32 bytes which are 16-byte aligned before we heap allocate
typedef TAlignedBytes<16, 16> FAlignedInlineDelegateType;
#if USE_SMALL_DELEGATES
typedef FHeapAllocator FDelegateAllocatorType;
#else
typedef TInlineAllocator<2> FDelegateAllocatorType;
#endif
#else
// ... except on Win32, because we can't pass 16-byte aligned types by value, as some delegates are
// so we'll just keep it heap-allocated, which are always sufficiently aligned.
typedef TAlignedBytes<16, 8> FAlignedInlineDelegateType;
typedef FHeapAllocator FDelegateAllocatorType;
#endif

class FDelegateBase
{
public:
	FDelegateAllocatorType::ForElementType<FAlignedInlineDelegateType> DelegateAllocator;
	int32 DelegateSize;
};
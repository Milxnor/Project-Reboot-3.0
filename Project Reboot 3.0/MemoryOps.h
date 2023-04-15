#pragma once

#include "inc.h"

#include "EnableIf.h"
#include "UnrealTypeTraits.h"

template <typename DestinationElementType, typename SourceElementType, typename SizeType>
FORCEINLINE typename TEnableIf<!TIsBitwiseConstructible<DestinationElementType, SourceElementType>::Value>::Type ConstructItems(void* Dest, const SourceElementType* Source, SizeType Count, int ElementSize = sizeof(SourceElementType))
{
	while (Count)
	{
		new (Dest) DestinationElementType(*Source);
		++(DestinationElementType*&)Dest;
		++Source;
		--Count;
	}
}

template <typename DestinationElementType, typename SourceElementType, typename SizeType>
FORCEINLINE typename TEnableIf<TIsBitwiseConstructible<DestinationElementType, SourceElementType>::Value>::Type ConstructItems(void* Dest, const SourceElementType* Source, SizeType Count, int ElementSize = sizeof(SourceElementType))
{
	// FMemory::Memcpy(Dest, Source, ElementSize * Count);
	memcpy(Dest, Source, ElementSize);
}
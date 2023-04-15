#pragma once

#include "IntroSort.h"

#include "UnrealTemplate.h"

namespace Algo
{
	template <typename RangeType, typename PredicateType>
	FORCEINLINE void Sort(RangeType& Range, PredicateType Pred)
	{
		IntroSort(Range, MoveTemp(Pred));
	}
}
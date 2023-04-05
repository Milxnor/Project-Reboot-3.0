#pragma once

#include "GenericPlatformMath.h"

struct FMath : public FPlatformMath
{
	template< class T >
	static FORCEINLINE T Clamp(const T X, const T Min, const T Max)
	{
		return X < Min ? Min : X < Max ? X : Max;
	}
};
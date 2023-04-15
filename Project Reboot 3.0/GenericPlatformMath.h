#pragma once

#include "inc.h"

class FPlatformMath
{
public:
	static constexpr FORCEINLINE int32 TruncToInt(float F)
	{
		return (int32)F;
	}

	static constexpr FORCEINLINE float TruncToFloat(float F)
	{
		return (float)TruncToInt(F);
	}

	template< class T >
	static constexpr FORCEINLINE T Min(const T A, const T B)
	{
		return (A <= B) ? A : B;
	}

	static FORCEINLINE int32 FloorToInt(float F)
	{
		return TruncToInt(floorf(F));
	}

	template< class T, class U >
	static FORCEINLINE T Lerp(const T& A, const T& B, const U& Alpha)
	{
		return (T)(A + Alpha * (B - A));
	}

	static FORCEINLINE float Loge(float Value) { return logf(Value); }

	template< class T >
	static constexpr FORCEINLINE T Max(const T A, const T B)
	{
		return (A >= B) ? A : B;
	}

	static FORCEINLINE float FloorToFloat(float F)
	{
		return floorf(F);
	}

	static FORCEINLINE double FloorToDouble(double F)
	{
		return floor(F);
	}

	static FORCEINLINE int32 RoundToInt(float F)
	{
		return FloorToInt(F + 0.5f);
	}

	static FORCEINLINE float Fractional(float Value)
	{
		return Value - TruncToFloat(Value);
	}

	static FORCEINLINE double TruncToDouble(double F)
	{
		return trunc(F);
	}

	static FORCEINLINE double Fractional(double Value)
	{
		return Value - TruncToDouble(Value);
	}
};
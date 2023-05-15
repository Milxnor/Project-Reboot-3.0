#pragma once

#include "inc.h"

class FGenericPlatformMath
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

	static FORCEINLINE float InvSqrt(float F)
	{
		return 1.0f / sqrtf(F);
	}

	static FORCENOINLINE float Fmod(float X, float Y);

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

	static FORCEINLINE float Sin(float Value) { return sinf(Value); }
	static FORCEINLINE float Asin(float Value) { return asinf((Value < -1.f) ? -1.f : ((Value < 1.f) ? Value : 1.f)); }
	static FORCEINLINE float Sinh(float Value) { return sinhf(Value); }
	static FORCEINLINE float Cos(float Value) { return cosf(Value); }
	static FORCEINLINE float Acos(float Value) { return acosf((Value < -1.f) ? -1.f : ((Value < 1.f) ? Value : 1.f)); }
	static FORCEINLINE float Tan(float Value) { return tanf(Value); }
	static FORCEINLINE float Atan(float Value) { return atanf(Value); }
	static float Atan2(float Y, float X);
	static FORCEINLINE float Sqrt(float Value) { return sqrtf(Value); }
	static FORCEINLINE float Pow(float A, float B) { return powf(A, B); }

	template< class T >
	static constexpr FORCEINLINE T Abs(const T A)
	{
		return (A >= (T)0) ? A : -A;
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
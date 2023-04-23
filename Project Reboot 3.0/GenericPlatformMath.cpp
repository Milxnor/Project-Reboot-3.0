#include "GenericPlatformMath.h"

#include "UnrealMathUtility.h"

#define PI 					(3.1415926535897932f)

/*FORCENOINLINE*/ float FGenericPlatformMath::Fmod(float X, float Y)
{
	if (fabsf(Y) <= 1.e-8f)
	{
		// FmodReportError(X, Y);
		return 0.f;
	}
	const float Div = (X / Y);
	// All floats where abs(f) >= 2^23 (8388608) are whole numbers so do not need truncation, and avoid overflow in TruncToFloat as they get even larger.
	const float Quotient = fabsf(Div) < 8388608.f ? TruncToFloat(Div) : Div;
	float IntPortion = Y * Quotient;

	// Rounding and imprecision could cause IntPortion to exceed X and cause the result to be outside the expected range.
	// For example Fmod(55.8, 9.3) would result in a very small negative value!
	if (fabsf(IntPortion) > fabsf(X))
	{
		IntPortion = X;
	}

	const float Result = X - IntPortion;
	return Result;
}

float FGenericPlatformMath::Atan2(float Y, float X)
{
	//return atan2f(Y,X);
	// atan2f occasionally returns NaN with perfectly valid input (possibly due to a compiler or library bug).
	// We are replacing it with a minimax approximation with a max relative error of 7.15255737e-007 compared to the C library function.
	// On PC this has been measured to be 2x faster than the std C version.

	const float absX = FMath::Abs(X);
	const float absY = FMath::Abs(Y);
	const bool yAbsBigger = (absY > absX);
	float t0 = yAbsBigger ? absY : absX; // Max(absY, absX)
	float t1 = yAbsBigger ? absX : absY; // Min(absX, absY)

	if (t0 == 0.f)
		return 0.f;

	float t3 = t1 / t0;
	float t4 = t3 * t3;

	static const float c[7] = {
		+7.2128853633444123e-03f,
		-3.5059680836411644e-02f,
		+8.1675882859940430e-02f,
		-1.3374657325451267e-01f,
		+1.9856563505717162e-01f,
		-3.3324998579202170e-01f,
		+1.0f
	};

	t0 = c[0];
	t0 = t0 * t4 + c[1];
	t0 = t0 * t4 + c[2];
	t0 = t0 * t4 + c[3];
	t0 = t0 * t4 + c[4];
	t0 = t0 * t4 + c[5];
	t0 = t0 * t4 + c[6];
	t3 = t0 * t3;

	t3 = yAbsBigger ? (0.5f * PI) - t3 : t3;
	t3 = (X < 0.0f) ? PI - t3 : t3;
	t3 = (Y < 0.0f) ? -t3 : t3;

	return t3;
}
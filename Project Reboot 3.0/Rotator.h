#pragma once

#include "Quat.h"
#include "Vector.h"

#include "UnrealMathUtility.h"

struct FRotator
{
#ifdef ABOVE_S20
	double Pitch;
	double Yaw;
	double Roll;
#else
	float Pitch;
	float Yaw;
	float Roll;
#endif

	FQuat Quaternion() const;

	FVector Vector() const;

	static float NormalizeAxis(float Angle);
	static float ClampAxis(float Angle);
};

FORCEINLINE float FRotator::ClampAxis(float Angle)
{
	// returns Angle in the range (-360,360)
	Angle = FMath::Fmod(Angle, 360.f);

	if (Angle < 0.f)
	{
		// shift to [0,360) range
		Angle += 360.f;
	}

	return Angle;
}

FORCEINLINE float FRotator::NormalizeAxis(float Angle)
{
	// returns Angle in the range [0,360)
	Angle = ClampAxis(Angle);

	if (Angle > 180.f)
	{
		// shift to (-180,180]
		Angle -= 360.f;
	}

	return Angle;
}
#pragma once

#include "Quat.h"
#include "Vector.h"

struct FRotator
{
	float Pitch;
	float Yaw;
	float Roll;

	FQuat Quaternion();

	FVector Vector() const;
};
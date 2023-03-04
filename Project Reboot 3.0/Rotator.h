#pragma once

#include "Quat.h"

struct FRotator
{
	float Pitch;
	float Yaw;
	float Roll;

	FQuat Quaternion();
};
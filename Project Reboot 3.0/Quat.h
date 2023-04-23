#pragma once

#include "inc.h"

MS_ALIGN(16) struct FQuat
{
public:

	/** The quaternion's X-component. */
	float X;

	/** The quaternion's Y-component. */
	float Y;

	/** The quaternion's Z-component. */
	float Z;

	/** The quaternion's W-component. */
	float W;

	struct FRotator Rotator() const;
};
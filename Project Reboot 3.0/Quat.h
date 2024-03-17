#pragma once

#include "inc.h"

MS_ALIGN(16) struct FQuat
{
public:

#ifndef ABOVE_S20
	/** The quaternion's X-component. */
	float X;

	/** The quaternion's Y-component. */
	float Y;

	/** The quaternion's Z-component. */
	float Z;

	/** The quaternion's W-component. */
	float W;
#else
	double                                        X;                                                 // 0x0000(0x0008)(Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	double                                        Y;                                                 // 0x0008(0x0008)(Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	double                                        Z;                                                 // 0x0010(0x0008)(Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	double                                        W;
#endif

	struct FRotator Rotator() const;
};
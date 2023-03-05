#pragma once

#include "Object.h"

class UKismetMathLibrary : public UObject
{
public:
	static float RandomFloatInRange(float min, float max);

	static UClass* StaticClass();
};
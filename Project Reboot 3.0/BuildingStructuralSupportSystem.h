#pragma once

#include "Object.h"
#include "Vector.h"

class UBuildingStructuralSupportSystem : public UObject
{
public:
	bool IsWorldLocValid(const FVector& WorldLoc);
};
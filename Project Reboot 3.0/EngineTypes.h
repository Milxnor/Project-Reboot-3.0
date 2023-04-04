#pragma once

#include "Object.h"

enum class ESpawnActorCollisionHandlingMethod : uint8
{
	Undefined,
	AlwaysSpawn,
	AdjustIfPossibleButAlwaysSpawn,
	AdjustIfPossibleButDontSpawnIfColliding,
	DontSpawnIfColliding
};

struct FHitResult
{
	static class UStruct* GetStruct();
	static int GetStructSize();
};
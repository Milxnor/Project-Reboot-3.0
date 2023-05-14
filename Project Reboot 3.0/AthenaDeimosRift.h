#pragma once

#include "BuildingRift.h"
#include "Stack.h"

class AAthenaDeimosRift : public ABuildingRift
{
public:
	static inline void (*QueueActorsToSpawnOriginal)(UObject* Context, FFrame* Stack, void* Ret);

	static void QueueActorsToSpawnHook(UObject* Context, FFrame* Stack, void* Ret);
};
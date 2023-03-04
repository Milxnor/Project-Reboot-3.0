#pragma once

#include "World.h"
#include "NetDriver.h"

#include "UnrealString.h"

class UEngine : public UObject
{
public:
	static inline UNetDriver* (*Engine_CreateNetDriver)(UEngine* Engine, UWorld* InWorld, FName NetDriverDefinition);

	UNetDriver* CreateNetDriver(UWorld* InWorld, FName NetDriverDefinition) { return Engine_CreateNetDriver(this, InWorld, NetDriverDefinition); }
};

// static inline bool (*CreateNamedNetDriver)(UWorld* InWorld, FName NetDriverName, FName NetDriverDefinition);
// static inline UNetDriver* CreateNetDriver_Local(UEngine* Engine, FWorldContext& Context, FName NetDriverDefinition)
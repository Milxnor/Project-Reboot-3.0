#pragma once

#include "ActorComponent.h"

#include "reboot.h"

class UAthenaResurrectionComponent : public UActorComponent
{
public:
	TWeakObjectPtr<AActor>& GetResurrectionLocation()
	{
		static auto ResurrectionLocationOffset = GetOffset("ResurrectionLocation");
		return Get<TWeakObjectPtr<AActor>>(ResurrectionLocationOffset);
	}

	int& GetClosestSpawnMachineIndex()
	{
		static auto ClosestSpawnMachineIndexOffset = GetOffset("ClosestSpawnMachineIndex");
		return Get<int>(ClosestSpawnMachineIndexOffset);
	}
};
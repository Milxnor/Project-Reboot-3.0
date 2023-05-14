#pragma once

#include "reboot.h"

#include "GameplayStatics.h"

class ABP_IslandScripting_C : public AActor // AFortAlwaysRelevantReplicatedActor
{
public:
	bool& IsDeimosActive()
	{
		static auto IsDeimosActiveOffset = GetOffset("IsDeimosActive");
		return Get<bool>(IsDeimosActiveOffset);
	}

	void OnRep_IsDeimosActive()
	{
		static auto OnRep_IsDeimosActiveFn = FindObject<UFunction>("/Game/Athena/Prototype/Blueprints/Island/BP_IslandScripting.BP_IslandScripting_C.OnRep_IsDeimosActive");
		this->ProcessEvent(OnRep_IsDeimosActiveFn);
	}

	void OnRep_UpdateMap()
	{
		static auto OnRep_UpdateMapFn = FindObject<UFunction>("/Game/Athena/Prototype/Blueprints/Island/BP_IslandScripting.BP_IslandScripting_C.OnRep_UpdateMap");
		this->ProcessEvent(OnRep_UpdateMapFn);
	}

	void Initialize();

	static ABP_IslandScripting_C* GetIslandScripting();

	static UClass* StaticClass()
	{
		/* static */ auto Class = FindObject<UClass>("/Game/Athena/Prototype/Blueprints/Island/BP_IslandScripting.BP_IslandScripting_C");
		return Class;
	}
};
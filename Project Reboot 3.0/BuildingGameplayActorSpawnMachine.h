#pragma once

#include "BuildingGameplayActor.h"
#include "OnlineReplStructs.h"
#include "WeakObjectPtr.h"

class ABuildingGameplayActorSpawnMachine : public ABuildingGameplayActor
{
public:
	TArray<FUniqueNetIdRepl>& GetPlayerIdsForResurrection()
	{
		static auto PlayerIdsForResurrectionOffset = GetOffset("PlayerIdsForResurrection");
		return Get<TArray<FUniqueNetIdRepl>>(PlayerIdsForResurrectionOffset);
	}

	AActor*& GetResurrectLocation() // actually AFortPlayerStart
	{
		static auto ResurrectLocationOffset = GetOffset("ResurrectLocation");
		return Get<AActor*>(ResurrectLocationOffset);
	}

	uint8& GetActiveTeam()
	{
		static auto ActiveTeamOffset = GetOffset("ActiveTeam");
		return Get<uint8>(ActiveTeamOffset);
	}

	uint8& GetSquadId()
	{
		static auto SquadIdOffset = GetOffset("SquadId");
		return Get<uint8>(SquadIdOffset);
	}

	TWeakObjectPtr<class AFortPlayerControllerAthena> GetInstigatorPC()
	{
		static auto InstigatorPCOffset = GetOffset("InstigatorPC");
		return Get<TWeakObjectPtr<class AFortPlayerControllerAthena>>(InstigatorPCOffset);
	}

	void FinishResurrection(int SquadId);

	static void RebootingDelegateHook(ABuildingGameplayActorSpawnMachine* SpawnMachine);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.BuildingGameplayActorSpawnMachine");
		return Class;
	}
};
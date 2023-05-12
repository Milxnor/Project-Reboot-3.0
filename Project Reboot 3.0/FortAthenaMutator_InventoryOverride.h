#pragma once

#include "FortAthenaMutator.h"

enum class EAthenaLootDropOverride : uint8_t
{
	NoOverride = 0,
	ForceDrop = 1,
	ForceKeep = 2,
	ForceDestroy = 3,
	ForceDropUnlessRespawning = 4,
	ForceDestroyUnlessRespawning = 5,
	EAthenaLootDropOverride_MAX = 6
};

enum class EAthenaInventorySpawnOverride : uint8_t
{
	NoOverride = 0,
	Always = 1,
	IntialSpawn = 2,
	AircraftPhaseOnly = 3,
	EAthenaInventorySpawnOverride_MAX = 4
};

struct FItemLoadoutContainer
{
	TArray<FItemAndCount> Loadout;
};

struct FItemLoadoutTeamMap
{
	unsigned char                                      TeamIndex;                                                // 0x0000(0x0001) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      LoadoutIndex;                                             // 0x0001(0x0001) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	EAthenaInventorySpawnOverride                      UpdateOverrideType;                                       // 0x0002(0x0001) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	EAthenaLootDropOverride                            DropAllItemsOverride;                                     // 0x0003(0x0001) (Edit, BlueprintVisible, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

class AFortAthenaMutator_InventoryOverride : public AFortAthenaMutator
{
public:
	TArray<FItemLoadoutContainer>& GetInventoryLoadouts()
	{
		static auto InventoryLoadoutsOffset = GetOffset("InventoryLoadouts");
		return Get<TArray<FItemLoadoutContainer>>(InventoryLoadoutsOffset);
	}

	TArray<FItemLoadoutTeamMap>* GetTeamLoadouts()
	{
		static auto TeamLoadoutsOffset = GetOffset("TeamLoadouts", false);

		if (TeamLoadoutsOffset == -1)
			return nullptr;

		return GetPtr<TArray<FItemLoadoutTeamMap>>(TeamLoadoutsOffset);
	}

	FItemLoadoutTeamMap GetLoadoutTeamForTeamIndex(uint8_t TeamIndex)
	{
		auto TeamLoadouts = GetTeamLoadouts();

		if (!TeamLoadouts)
			return FItemLoadoutTeamMap();

		for (int i = 0; i < TeamLoadouts->Num(); i++)
		{
			auto& TeamLoadout = TeamLoadouts->at(i);

			if (TeamLoadout.TeamIndex == TeamIndex)
				return TeamLoadout;
		}

		return FItemLoadoutTeamMap();
	}

	FItemLoadoutContainer GetLoadoutContainerForTeamIndex(uint8_t TeamIndex)
	{
		auto LoadoutTeam = GetLoadoutTeamForTeamIndex(TeamIndex);

		if (LoadoutTeam.TeamIndex == TeamIndex)
		{
			auto& InventoryLoadouts = GetInventoryLoadouts();

			if (InventoryLoadouts.Num() - 1 < LoadoutTeam.LoadoutIndex)
				return FItemLoadoutContainer();

			return InventoryLoadouts.at(LoadoutTeam.LoadoutIndex);
		}

		return FItemLoadoutContainer();
	}

	EAthenaInventorySpawnOverride& GetInventoryUpdateOverride(uint8_t TeamIndex = 255)
	{
		if (TeamIndex != 255)
		{
			auto LoadoutTeam = GetLoadoutTeamForTeamIndex(TeamIndex);

			if (LoadoutTeam.TeamIndex == TeamIndex)
			{
				if (LoadoutTeam.UpdateOverrideType != EAthenaInventorySpawnOverride::NoOverride)
					return LoadoutTeam.UpdateOverrideType;
			}
		}

		static auto InventoryUpdateOverrideOffset = GetOffset("InventoryUpdateOverride");
		return Get<EAthenaInventorySpawnOverride>(InventoryUpdateOverrideOffset);
	}

	EAthenaLootDropOverride& GetDropAllItemsOverride(uint8_t TeamIndex = 255)
	{
		if (TeamIndex != 255)
		{
			auto LoadoutTeam = GetLoadoutTeamForTeamIndex(TeamIndex);

			if (LoadoutTeam.TeamIndex == TeamIndex)
			{
				if (LoadoutTeam.DropAllItemsOverride != EAthenaLootDropOverride::NoOverride)
					return LoadoutTeam.DropAllItemsOverride;
			}
		}

		static auto DropAllItemsOverrideOffset = GetOffset("DropAllItemsOverride");
		return Get<EAthenaLootDropOverride>(DropAllItemsOverrideOffset);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaMutator_InventoryOverride");
		return Class;
	}
};
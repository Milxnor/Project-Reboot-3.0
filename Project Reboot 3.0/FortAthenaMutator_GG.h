#pragma once

#include "Actor.h"
#include "CurveTable.h"
#include "GameplayAbilityTypes.h"
#include "FortWeaponItemDefinition.h"
#include "Stack.h"
#include "FortPlayerStateAthena.h"
#include "FortAthenaMutator.h"

struct FGunGameGunEntry
{
	UFortWeaponItemDefinition* Weapon;                                                   // 0x0000(0x0008) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FScalableFloat                              Enabled;                                                  // 0x0008(0x0020) (Edit, DisableEditOnInstance, NativeAccessSpecifierPublic)
	FScalableFloat                              AwardAtElim;                                              // 0x0028(0x0020) (Edit, DisableEditOnInstance, NativeAccessSpecifierPublic)
};

struct FGunGameGunEntries
{
	TArray<struct FGunGameGunEntry>                    Entries;                                                  // 0x0000(0x0010) (ZeroConstructor, NativeAccessSpecifierPublic)
};

struct FGunGamePlayerData
{
	TArray<class UFortWeaponItemDefinition*>           CurrentlyAssignedWeapons;                                 // 0x0000(0x0010) (ZeroConstructor, NativeAccessSpecifierPublic)
};

class AFortAthenaMutator_GG : public AFortAthenaMutator
{
public:
	TArray<FGunGameGunEntry>& GetWeaponEntries()
	{
		static auto WeaponEntriesOffset = GetOffset("WeaponEntries");
		return Get<TArray<FGunGameGunEntry>>(WeaponEntriesOffset);
	}

	TMap<int, FGunGameGunEntries>& GetAwardEntriesAtElimMap()
	{
		static auto AwardEntriesAtElimMapOffset = GetOffset("AwardEntriesAtElimMap");
		return Get<TMap<int, FGunGameGunEntries>>(AwardEntriesAtElimMapOffset);
	}

	TMap<AFortPlayerStateAthena*, FGunGamePlayerData>& GetPlayerData()
	{
		static auto PlayerDataOffset = GetOffset("PlayerData");
		return Get<TMap<AFortPlayerStateAthena*, FGunGamePlayerData>>(PlayerDataOffset);
	}

	FGunGameGunEntries GetEntriesFromAward(const FScalableFloat& AwardAtElim)
	{
		auto& AwardEntriesAtElimMap = GetAwardEntriesAtElimMap();

		float Value = 0;

		for (auto& AwardEntry : AwardEntriesAtElimMap)
		{
			if (AwardEntry.First == Value)
			{
				return AwardEntry.Second;
			}
		}
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaMutator_GG");
		return Class;
	}
};
#pragma once

#include "BuildingActor.h"
#include "PlayerState.h"
#include "CurveTable.h"

enum class EFortResourceType : uint8_t
{
	Wood = 0,
	Stone = 1,
	Metal = 2,
	Permanite = 3,
	None = 4,
	EFortResourceType_MAX = 5
};

using UStaticMesh = UObject;
using UStaticMeshComponent = UObject;
using UParticleSystem = UObject;

struct FMeshSet
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.MeshSet");
		return Struct;
	}

	static int GetPropertiesSize() { return GetStruct()->GetPropertiesSize(); }

	UStaticMesh* GetBaseMesh()
	{
		static auto BaseMeshOffset = FindOffsetStruct("/Script/FortniteGame.MeshSet", "BaseMesh");
		return *(UStaticMesh**)(__int64(this) + BaseMeshOffset);
	}

	UStaticMesh* GetSearchedMesh()
	{
		static auto SearchedMeshOffset = FindOffsetStruct("/Script/FortniteGame.MeshSet", "SearchedMesh");
		return *(UStaticMesh**)(__int64(this) + SearchedMeshOffset);
	}

	EFortResourceType& GetResourceType()
	{
		static auto ResourceTypeOffset = FindOffsetStruct("/Script/FortniteGame.MeshSet", "ResourceType");
		return *(EFortResourceType*)(__int64(this) + ResourceTypeOffset);
	}

	float& GetLootNoiseRange()
	{
		static auto LootNoiseRangeOffset = FindOffsetStruct("/Script/FortniteGame.MeshSet", "LootNoiseRange");
		return *(float*)(__int64(this) + LootNoiseRangeOffset);
	}

	FVector& GetLootSpawnLocation()
	{
		static auto LootSpawnLocationOffset = FindOffsetStruct("/Script/FortniteGame.MeshSet", "LootSpawnLocation");
		return *(FVector*)(__int64(this) + LootSpawnLocationOffset);
	}

	float& GetLootNoiseLoudness()
	{
		static auto LootNoiseLoudnessOffset = FindOffsetStruct("/Script/FortniteGame.MeshSet", "LootNoiseLoudness");
		return *(float*)(__int64(this) + LootNoiseLoudnessOffset);
	}

	FCurveTableRowHandle& GetSearchSpeed()
	{
		static auto SearchSpeedOffset = FindOffsetStruct("/Script/FortniteGame.MeshSet", "SearchSpeed");
		return *(FCurveTableRowHandle*)(__int64(this) + SearchSpeedOffset);
	}

	UParticleSystem*& GetConstructedEffect()
	{
		static auto ConstructedEffectOffset = FindOffsetStruct("/Script/FortniteGame.MeshSet", "ConstructedEffect");
		return *(UParticleSystem**)(__int64(this) + ConstructedEffectOffset);
	}

	UParticleSystem*& GetBreakEffect()
	{
		static auto BreakEffectOffset = FindOffsetStruct("/Script/FortniteGame.MeshSet", "BreakEffect");
		return *(UParticleSystem**)(__int64(this) + BreakEffectOffset);
	}
};

struct FTierMeshSets
{
	int32& GetTier()
	{
		static auto TierOffset = FindOffsetStruct("/Script/FortniteGame.TierMeshSets", "Tier");
		return *(int32*)(__int64(this) + TierOffset);
	}

	TArray<FMeshSet>& GetMeshSets()
	{
		static auto MeshSetsOffset = FindOffsetStruct("/Script/FortniteGame.TierMeshSets", "MeshSets");
		return *(TArray<FMeshSet>*)(__int64(this) + MeshSetsOffset);
	}
};

class ABuildingSMActor : public ABuildingActor
{
public:
	bool IsPlayerPlaced()
	{
		static auto bPlayerPlacedOffset = GetOffset("bPlayerPlaced");
		static auto bPlayerPlacedFieldMask = GetFieldMask(this->GetProperty("bPlayerPlaced"));
		return ReadBitfieldValue(bPlayerPlacedOffset, bPlayerPlacedFieldMask);
	}

	TArray<FTierMeshSets>& GetAlternateMeshes()
	{
		static auto AlternateMeshesOffset = GetOffset("AlternateMeshes");
		return Get<TArray<FTierMeshSets>>(AlternateMeshesOffset);
	}

	int32& GetAltMeshIdx()
	{
		static auto AltMeshIdxOffset = GetOffset("AltMeshIdx");
		return Get<int32>(AltMeshIdxOffset);
	}

	void BuildingSMActor_SetMeshSet(FMeshSet* MeshSet);

	void SetPlayerPlaced(bool NewValue)
	{
		static auto bPlayerPlacedOffset = GetOffset("bPlayerPlaced");
		static auto bPlayerPlacedFieldMask = GetFieldMask(this->GetProperty("bPlayerPlaced"));
		this->SetBitfieldValue(bPlayerPlacedOffset, bPlayerPlacedFieldMask, NewValue);
	}

	APlayerState*& GetEditingPlayer()
	{
		static auto EditingPlayerOffset = GetOffset("EditingPlayer");
		return Get<APlayerState*>(EditingPlayerOffset);
	}

	int& GetCurrentBuildingLevel()
	{
		static auto CurrentBuildingLevelOffset = GetOffset("CurrentBuildingLevel");
		return Get<int>(CurrentBuildingLevelOffset);
	}

	UStaticMeshComponent*& GetStaticMeshComponent()
	{
		static auto StaticMeshComponentOffset = GetOffset("StaticMeshComponent");
		return Get<UStaticMeshComponent*>(StaticMeshComponentOffset);
	}

	EFortResourceType& GetResourceType()
	{
		static auto ResourceTypeOffset = GetOffset("ResourceType");
		return Get<EFortResourceType>(ResourceTypeOffset);
	}

	void SetEditingPlayer(APlayerState* NewEditingPlayer) // actually AFortPlayerStateZone
	{
		if (// AActor::HasAuthority() &&
			(!GetEditingPlayer() || !NewEditingPlayer)
			)
		{
			SetNetDormancy((ENetDormancy)(2 - (NewEditingPlayer != 0)));
			// they do something here
			GetEditingPlayer() = NewEditingPlayer;
		}
	}

	static UClass* StaticClass();
};
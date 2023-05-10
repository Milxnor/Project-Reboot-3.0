#pragma once

#include "FortAthenaMutator.h"
#include "GameplayAbilityTypes.h"
#include "CurveTable.h"
#include "FortWorldItemDefinition.h"
#include "FortInventory.h"

enum class ERespawnRequirements : uint8_t
{
	RespawnOnly = 0,
	NoRespawnOnly = 1,
	Both = 2,
	ERespawnRequirements_MAX = 3
};

struct FItemsToDropOnDeath
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.ItemsToDropOnDeath");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }
	
	UFortWorldItemDefinition*& GetItemToDrop()
	{
		static auto ItemToDropOffset = FindOffsetStruct("/Script/FortniteGame.ItemsToDropOnDeath", "ItemToDrop");
		return *(UFortWorldItemDefinition**)(__int64(this) + ItemToDropOffset);
	}

	FScalableFloat* GetNumberToDrop()
	{
		static auto NumberToDropOffset = FindOffsetStruct("/Script/FortniteGame.ItemsToDropOnDeath", "NumberToDrop");
		return (FScalableFloat*)(__int64(this) + NumberToDropOffset);
	}
};

class AFortAthenaMutator_ItemDropOnDeath : public AFortAthenaMutator
{
public:
	ERespawnRequirements/*&*/ GetRespawnRequirements()
	{
		static auto RespawnRequirementsOffset = GetOffset("RespawnRequirements");
		return Get<ERespawnRequirements>(RespawnRequirementsOffset);
	}

	TArray<FItemsToDropOnDeath>& GetItemsToDrop()
	{
		static auto ItemsToDropOffset = GetOffset("ItemsToDrop");
		return Get<TArray<FItemsToDropOnDeath>>(ItemsToDropOffset);
	}
};
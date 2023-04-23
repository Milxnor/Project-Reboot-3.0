#pragma once

#include "Actor.h"
#include "CurveTable.h"
#include "GameplayAbilityTypes.h"
#include "FortWorldItemDefinition.h"
#include "Stack.h"
#include "FortAthenaMutator.h"

struct FItemsToGive
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.ItemsToGive");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	UFortWorldItemDefinition*& GetItemToDrop()
	{
		static auto ItemToDropOffset = FindOffsetStruct("/Script/FortniteGame.ItemsToGive", "ItemToDrop");
		return *(UFortWorldItemDefinition**)(__int64(this) + ItemToDropOffset);
	}

	FScalableFloat& GetNumberToGive()
	{
		static auto NumberToGiveOffset = FindOffsetStruct("/Script/FortniteGame.ItemsToGive", "NumberToGive");
		return *(FScalableFloat*)(__int64(this) + NumberToGiveOffset);
	}
};

class AFortAthenaMutator_GiveItemsAtGamePhaseStep : public AFortAthenaMutator
{
public:
	static inline void (*OnGamePhaseStepChangedOriginal)(UObject* Context, FFrame& Stack, void* Ret);

	uint8_t& GetPhaseToGiveItems()
	{
		static auto PhaseToGiveItemsOffset = GetOffset("PhaseToGiveItems");
		return Get<uint8_t>(PhaseToGiveItemsOffset);
	}

	TArray<FItemsToGive>& GetItemsToGive()
	{
		static auto ItemsToGiveOffset = GetOffset("ItemsToGive");
		return Get<TArray<FItemsToGive>>(ItemsToGiveOffset);
	}

	static void OnGamePhaseStepChangedHook(UObject* Context, FFrame& Stack, void* Ret);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaMutator_GiveItemsAtGamePhaseStep");
		return Class;
	}
};
#pragma once

#include "Actor.h"
#include "CurveTable.h"
#include "GameplayAbilityTypes.h"
#include "FortWorldItemDefinition.h"
#include "Stack.h"
#include "FortAthenaMutator.h"

struct FItemsToGive
{
	UFortWorldItemDefinition* ItemToDrop;                                               // 0x0000(0x0008) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FScalableFloat                              NumberToGive;                                             // 0x0008(0x0020) (Edit, NativeAccessSpecifierPublic)
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
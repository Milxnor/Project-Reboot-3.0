#pragma once

#include "FortAthenaMutator.h"
#include "GameplayAbilityTypes.h"
#include "CurveTable.h"
#include "FortWorldItemDefinition.h"
#include "FortInventory.h"

struct FItemsToDropOnDeath
{
	UFortWorldItemDefinition* ItemToDrop;                                               // 0x0000(0x0008) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FScalableFloat                              NumberToDrop;                                             // 0x0008(0x0020) (Edit, NativeAccessSpecifierPublic)
};

class AFortAthenaMutator_ItemDropOnDeath : public AFortAthenaMutator
{
public:
	TArray<FItemsToDropOnDeath>& GetItemsToDrop()
	{
		static auto ItemsToDropOffset = GetOffset("ItemsToDrop");
		return Get<TArray<FItemsToDropOnDeath>>(ItemsToDropoOffset);
	}
};
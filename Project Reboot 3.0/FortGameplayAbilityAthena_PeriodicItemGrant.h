#pragma once

#include "Object.h"
#include "Stack.h"

#include "GameplayAbilityTypes.h"
#include "FortWorldItemDefinition.h"

struct FActiveItemGrantInfo
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.ActiveItemGrantInfo");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	UFortWorldItemDefinition*& GetItem()
	{
		static auto ItemOffset = FindOffsetStruct("/Script/FortniteGame.ActiveItemGrantInfo", "Item");
		return *(UFortWorldItemDefinition**)(__int64(this) + ItemOffset);
	}

	FScalableFloat& GetAmountToGive()
	{
		static auto AmountToGiveOffset = FindOffsetStruct("/Script/FortniteGame.ActiveItemGrantInfo", "AmountToGive");
		return *(FScalableFloat*)(__int64(this) + AmountToGiveOffset);
	}

	FScalableFloat& GetMaxAmount()
	{
		static auto MaxAmountOffset = FindOffsetStruct("/Script/FortniteGame.ActiveItemGrantInfo", "MaxAmount");
		return *(FScalableFloat*)(__int64(this) + MaxAmountOffset);
	}
};

class UFortGameplayAbilityAthena_PeriodicItemGrant : public UObject // UFortGameplayAbility
{
public:
	static inline void (*StopItemAwardTimersOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*StartItemAwardTimersOriginal)(UObject* Context, FFrame& Stack, void* Ret);

	TMap<FActiveItemGrantInfo, FScalableFloat>& GetItemsToGrant()
	{
		static auto ItemsToGrantOffset = GetOffset("ItemsToGrant");
		return Get<TMap<FActiveItemGrantInfo, FScalableFloat>>(ItemsToGrantOffset);
	}

	TArray<FTimerHandle>& GetActiveTimers()
	{
		static auto ActiveTimersOffset = GetOffset("ActiveTimers");
		return Get<TArray<FTimerHandle>>(ActiveTimersOffset);
	}

	static void StopItemAwardTimersHook(UObject* Context, FFrame& Stack, void* Ret);
	static void StartItemAwardTimersHook(UObject* Context, FFrame& Stack, void* Ret);
};
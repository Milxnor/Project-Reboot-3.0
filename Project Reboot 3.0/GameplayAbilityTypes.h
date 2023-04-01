#pragma once

#include "Array.h"
#include "Map.h"
#include "GameplayAbilitySpec.h"

struct FGameplayAbilitySpecHandleAndPredictionKey
{
	FGameplayAbilitySpecHandle AbilityHandle;
	int32 PredictionKeyAtCreation;
};

/*struct FGameplayAbilityReplicatedDataContainer
{
	typedef TPair<FGameplayAbilitySpecHandleAndPredictionKey, TSharedRef<FAbilityReplicatedDataCache>> FKeyDataPair;

	TArray<FKeyDataPair> InUseData;
	TArray<TSharedRef<FAbilityReplicatedDataCache>> FreeData;
}; */
#pragma once

#include "Array.h"
#include "Map.h"
#include "CurveTable.h"
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

struct FScalableFloat // todo check where this actually goes
{
	float& GetValue()
	{
		static auto FloatOffset = FindOffsetStruct("/Script/GameplayAbilities.ScalableFloat", "Value");
		return *(float*)(__int64(this) + FloatOffset);
	}

	FCurveTableRowHandle& GetCurve()
	{
		static auto CurveOffset = FindOffsetStruct("/Script/GameplayAbilities.ScalableFloat", "Curve");
		return *(FCurveTableRowHandle*)(__int64(this) + CurveOffset);
	}
};
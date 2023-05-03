#include "FortGameplayAbilityAthena_PeriodicItemGrant.h"

void UFortGameplayAbilityAthena_PeriodicItemGrant::StopItemAwardTimersHook(UObject* Context, FFrame& Stack, void* Ret)
{
	// (Milxnor) We need clear all the timers in ActiveTimers.

	return StopItemAwardTimersOriginal(Context, Stack, Ret);
}

void UFortGameplayAbilityAthena_PeriodicItemGrant::StartItemAwardTimersHook(UObject* Context, FFrame& Stack, void* Ret)
{
	// (Milxnor) We need to loop through ItemsToGrant, and then using the Pair.Value we set a timer and then the Pair.Key for the item to grant, then we add the timer to ActiveTimers.

	return StartItemAwardTimersOriginal(Context, Stack, Ret);
}
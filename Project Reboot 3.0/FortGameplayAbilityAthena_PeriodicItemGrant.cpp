#include "FortGameplayAbilityAthena_PeriodicItemGrant.h"

void UFortGameplayAbilityAthena_PeriodicItemGrant::StartItemAwardTimersHook(UObject* Context, FFrame& Stack, void* Ret)
{
	LOG_INFO(LogDev, "StartItemAwardTimersHook!");
	return StartItemAwardTimersOriginal(Context, Stack, Ret);
}
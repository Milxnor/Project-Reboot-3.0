#pragma once

#include "Object.h"
#include "Stack.h"

class UFortGameplayAbilityAthena_PeriodicItemGrant : public UObject // UFortGameplayAbility
{
public:
	static inline void (*StartItemAwardTimersOriginal)(UObject* Context, FFrame& Stack, void* Ret);

	static void StartItemAwardTimersHook(UObject* Context, FFrame& Stack, void* Ret);
};
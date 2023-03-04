#pragma once

#include "PlayerState.h"
#include "AbilitySystemComponent.h"

class AFortPlayerState : public APlayerState
{
public:
	UAbilitySystemComponent*& GetAbilitySystemComponent()
	{
		static auto AbilitySystemComponentOffset = GetOffset("AbilitySystemComponent");
		return this->Get<UAbilitySystemComponent*>(AbilitySystemComponentOffset);
	}
};
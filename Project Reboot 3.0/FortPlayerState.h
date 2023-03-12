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

	int& GetWorldPlayerId()
	{
		static auto WorldPlayerIdOffset = GetOffset("WorldPlayerId");
		return this->Get<int>(WorldPlayerIdOffset);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortPlayerState");
		return Class;
	}
};
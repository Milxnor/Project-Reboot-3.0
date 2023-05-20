#pragma once

#include "OnlineReplStructs.h"
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

	void EndDBNOAbilities();

	static bool AreUniqueIDsIdentical(FUniqueNetIdRepl* A, FUniqueNetIdRepl* B);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortPlayerState");
		return Class;
	}
};
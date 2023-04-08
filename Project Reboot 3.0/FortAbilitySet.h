#pragma once

#include "AbilitySystemComponent.h"
#include "reboot.h"
#include "SoftObjectPtr.h"

struct FGameplayEffectApplicationInfoHard
{
public:
	UClass*           GameplayEffect;                                    // 0x0(0x8)(Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                        Level;
};

struct FGameplayEffectApplicationInfo
{
	TSoftObjectPtr<UClass>         GameplayEffect;                                  // 0x0000(0x0028) UNKNOWN PROPERTY: SoftClassProperty FortniteGame.GameplayEffectApplicationInfo.GameplayEffect
	float                                              Level;                                                    // 0x0028(0x0004) (Edit, ZeroConstructor, DisableEditOnInstance, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData01[0x4];                                       // 0x002C(0x0004) MISSED OFFSET
};

class UFortAbilitySet : public UObject
{
public:
	void GiveToAbilitySystem(UAbilitySystemComponent* AbilitySystemComponent, UObject* SourceObject = nullptr)
	{
		static auto GameplayAbilitiesOffset = this->GetOffset("GameplayAbilities");
		auto GameplayAbilities = this->GetPtr<TArray<UClass*>>(GameplayAbilitiesOffset);

		for (int i = 0; i < GameplayAbilities->Num(); i++)
		{
			UClass* AbilityClass = GameplayAbilities->At(i);

			if (!AbilityClass)
				continue;

			LOG_INFO(LogDev, "Giving AbilityClass {}", AbilityClass->GetFullName());

			AbilitySystemComponent->GiveAbilityEasy(AbilityClass, SourceObject);
		}

		static auto GameplayEffectApplicationInfoHardStruct = FindObject<UStruct>("/Script/FortniteGame.GameplayEffectApplicationInfoHard");

		if (!GameplayEffectApplicationInfoHardStruct)
			return;

		static auto GameplayEffectApplicationInfoHardSize = GameplayEffectApplicationInfoHardStruct->GetPropertiesSize();
		static auto GrantedGameplayEffectsOffset = this->GetOffset("GrantedGameplayEffects");
		auto GrantedGameplayEffects = this->GetPtr<TArray<FGameplayEffectApplicationInfoHard>>(GrantedGameplayEffectsOffset);

		for (int i = 0; i < GrantedGameplayEffects->Num(); i++)
		{
			auto& EffectToGrant = GrantedGameplayEffects->at(i, GameplayEffectApplicationInfoHardSize);

			if (!EffectToGrant.GameplayEffect)
			{
				continue;
			}

			LOG_INFO(LogDev, "Giving GameplayEffect {}", EffectToGrant.GameplayEffect->GetFullName());

			// UObject* GameplayEffect = EffectToGrant.GameplayEffect->CreateDefaultObject();
			FGameplayEffectContextHandle EffectContext{}; // AbilitySystemComponent->MakeEffectContext()
			AbilitySystemComponent->ApplyGameplayEffectToSelf(EffectToGrant.GameplayEffect, EffectToGrant.Level, EffectContext);
		}
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAbilitySet");
		return Class;
	}
};
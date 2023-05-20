#pragma once

#include "Class.h"
#include "NetSerialization.h"

#include "reboot.h"

struct FGameplayAbilitySpecHandle
{
	int Handle;

	/* void GenerateNewHandle()
	{
		if (true)
		{
			Handle = rand();
		}
		else
		{
			static int GHandle = 1;
			Handle = ++GHandle;
		}
	} */
};

struct FGameplayAbilityActivationInfo // TODO Move
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/GameplayAbilities.GameplayAbilityActivationInfo");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }
};

struct FGameplayAbilitySpec : FFastArraySerializerItem
{
	static int GetStructSize()
	{
		static auto GameplayAbilitySpecStruct = FindObject<UClass>("/Script/GameplayAbilities.GameplayAbilitySpec");
		static auto StructSize = GameplayAbilitySpecStruct->GetPropertiesSize();
			// *(int*)(__int64(GameplayAbilitySpecStruct) + Offsets::PropertiesSize);
		// LOG_INFO(LogAbilities, "StructSize: {}", StructSize);
		return StructSize;
	}

	UObject*& GetAbility()
	{
		static auto AbilityOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "Ability");
		return *(UObject**)(__int64(this) + AbilityOffset);
	}

	FGameplayAbilitySpecHandle& GetHandle()
	{
		static auto HandleOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "Handle");
		return *(FGameplayAbilitySpecHandle*)(__int64(this) + HandleOffset);
	}

	FGameplayAbilityActivationInfo* GetActivationInfo()
	{
		static auto ActivationInfoOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "ActivationInfo");
		return (FGameplayAbilityActivationInfo*)(__int64(this) + ActivationInfoOffset);
	}
};

static FGameplayAbilitySpec* MakeNewSpec(UClass* GameplayAbilityClass, UObject* SourceObject = nullptr, bool bAlreadyIsDefault = false)
{
	auto NewSpec = Alloc<FGameplayAbilitySpec>(FGameplayAbilitySpec::GetStructSize());

	if (!NewSpec)
		return nullptr;

	auto DefaultAbility = bAlreadyIsDefault ? GameplayAbilityClass : GameplayAbilityClass->CreateDefaultObject();

	static auto ActiveCountOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "ActiveCount", false);

	bool bUseNativeSpecConstructor = Addresses::SpecConstructor;

	if (bUseNativeSpecConstructor)
	{
		static __int64 (*SpecConstructor)(__int64 spec, UObject* Ability, int Level, int InputID, UObject* SourceObject) = decltype(SpecConstructor)(Addresses::SpecConstructor);

		SpecConstructor(__int64(NewSpec), DefaultAbility, 1, -1, SourceObject);
	}
	else
	{
		static auto LevelOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "Level");
		static auto SourceObjectOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "SourceObject");
		static auto InputIDOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "InputID");
		static auto GameplayEffectHandleOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "GameplayEffectHandle", false);

		((FFastArraySerializerItem*)NewSpec)->MostRecentArrayReplicationKey = -1;
		((FFastArraySerializerItem*)NewSpec)->ReplicationID = -1;
		((FFastArraySerializerItem*)NewSpec)->ReplicationKey = -1;

		NewSpec->GetHandle().Handle = rand(); // proper!
		NewSpec->GetAbility() = DefaultAbility;
		*(int*)(__int64(NewSpec) + LevelOffset) = 1;
		*(int*)(__int64(NewSpec) + InputIDOffset) = -1;
		*(UObject**)(__int64(NewSpec) + SourceObjectOffset) = SourceObject;
	}

	return NewSpec;
}
#pragma once

#include "Class.h"
#include "NetSerialization.h"

#include "reboot.h"

struct FGameplayAbilitySpecHandle
{
	int Handle;

	void GenerateNewHandle()
	{
		static int GHandle = 1;
		Handle = ++GHandle;
	}
};

struct FGameplayAbilitySpec : FFastArraySerializerItem
{
	static int GetStructSize()
	{
		static auto GameplayAbilitySpecStruct = FindObject<UClass>("/Script/GameplayAbilities.GameplayAbilitySpec");
		static auto StructSize = GameplayAbilitySpecStruct->GetPropertiesSize();
			// *(int*)(__int64(GameplayAbilitySpecStruct) + Offsets::PropertiesSize);
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
};

static FGameplayAbilitySpec* MakeNewSpec(UClass* GameplayAbilityClass, UObject* SourceObject = nullptr, bool bAlreadyIsDefault = false)
{
	auto NewSpec = Alloc<FGameplayAbilitySpec>(FGameplayAbilitySpec::GetStructSize());

	if (!NewSpec)
		return nullptr;

	static auto LevelOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "Level");
	static auto SourceObjectOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "SourceObject");
	static auto InputIDOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "InputID");

	((FFastArraySerializerItem*)NewSpec)->MostRecentArrayReplicationKey = -1;
	((FFastArraySerializerItem*)NewSpec)->ReplicationID = -1;
	((FFastArraySerializerItem*)NewSpec)->ReplicationKey = -1;

	NewSpec->GetHandle().GenerateNewHandle();
	NewSpec->GetAbility() = bAlreadyIsDefault ? GameplayAbilityClass : GameplayAbilityClass->CreateDefaultObject();
	*(int*)(__int64(NewSpec) + LevelOffset) = 0;
	*(int*)(__int64(NewSpec) + InputIDOffset) = -1;
	*(UObject**)(__int64(NewSpec) + SourceObjectOffset) = SourceObject;

	return NewSpec;
}
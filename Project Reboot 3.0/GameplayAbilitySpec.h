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
		static auto StructSize = 0x00C8;
		return StructSize;
	}

	UObject*& GetAbility()
	{
		static auto AbilityOffset = 0x0010;
		return *(UObject**)(__int64(this) + AbilityOffset);
	}

	FGameplayAbilitySpecHandle& GetHandle()
	{
		static auto HandleOffset = 0xC;
		return *(FGameplayAbilitySpecHandle*)(__int64(this) + HandleOffset);
	}
};

static FGameplayAbilitySpec* MakeNewSpec(UClass* GameplayAbilityClass, UObject* SourceObject = nullptr)
{
	auto NewSpec = Alloc<FGameplayAbilitySpec>(FGameplayAbilitySpec::GetStructSize());

	if (!NewSpec)
		return nullptr;

	static auto HandleOffset = 0xC;
	static auto AbilityOffset = 0x0010;
	static auto LevelOffset = 0x0018;
	static auto InputIDOffset = 0x001C;
	static auto SourceObjectOffset = 0x0020;

	((FFastArraySerializerItem*)NewSpec)->MostRecentArrayReplicationKey = -1;
	((FFastArraySerializerItem*)NewSpec)->ReplicationID = -1;
	((FFastArraySerializerItem*)NewSpec)->ReplicationKey = -1;

	NewSpec->GetHandle().GenerateNewHandle();
	NewSpec->GetAbility() = GameplayAbilityClass->CreateDefaultObject();
	*(int*)(__int64(NewSpec) + LevelOffset) = 0;
	*(int*)(__int64(NewSpec) + InputIDOffset) = -1;
	*(UObject**)(__int64(NewSpec) + SourceObjectOffset) = SourceObject;

	return NewSpec;
}
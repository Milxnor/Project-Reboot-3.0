#pragma once

#include "FortItemDefinition.h"
#include "FortLootLevel.h"

enum class EWorldItemDropBehavior : uint8_t
{
	DropAsPickup = 0,
	DestroyOnDrop = 1,
	DropAsPickupDestroyOnEmpty = 2,
	EWorldItemDropBehavior_MAX = 3
};

struct FFortLootLevelData : public FTableRowBase
{
public:
	FName                                  Category;                                          // 0x8(0x8)(Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int32                                        LootLevel;                                         // 0x10(0x4)(Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int32                                        MinItemLevel;                                      // 0x14(0x4)(Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int32                                        MaxItemLevel;                                      // 0x18(0x4)(Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	uint8                                        Pad_B94[0x4];                                      // Fixing Size Of Struct [ Dumper-7 ]
};

class UFortWorldItemDefinition : public UFortItemDefinition
{
public:
	bool CanBeDropped()
	{
		static auto bCanBeDroppedOffset = GetOffset("bCanBeDropped");
		static auto bCanBeDroppedFieldMask = GetFieldMask(GetProperty("bCanBeDropped"));
		return ReadBitfieldValue(bCanBeDroppedOffset, bCanBeDroppedFieldMask);
	}

	int& GetDropCount()
	{
		static auto DropCountOffset = GetOffset("DropCount");
		return Get<int>(DropCountOffset);
	}

	int PickLevel(int PreferredLevel) // well min level and maxlevel is sometimes in ufortowrlditemdeifnit9 then on older versions ufortitemdefinitoj so idk wher tyo put this
	{
		static auto MinLevelOffset = GetOffset("MinLevel");
		static auto MaxLevelOffset = GetOffset("MaxLevel");

		const int MinLevel = Get<int>(MinLevelOffset);
		const int MaxLevel = Get<int>(MaxLevelOffset);

		int PickedLevel = 0;

		if (PreferredLevel >= MinLevel)
			PickedLevel = PreferredLevel;

		if (MaxLevel >= 0)
		{
			if (PickedLevel <= MaxLevel)
				return PickedLevel;
			return MaxLevel;
		}

		return PickedLevel;
	}

	FDataTableCategoryHandle& GetLootLevelData()
	{
		static auto LootLevelDataOffset = GetOffset("LootLevelData");
		return Get<FDataTableCategoryHandle>(LootLevelDataOffset);
	}

	int GetFinalLevel(int WorldLevel)
	{
		auto ItemLevel = UFortLootLevel::GetItemLevel(GetLootLevelData(), WorldLevel);

		return PickLevel(ItemLevel >= 0 ? ItemLevel : 0);
	}

	EWorldItemDropBehavior& GetDropBehavior()
	{
		static auto DropBehaviorOffset = GetOffset("DropBehavior");
		return Get<EWorldItemDropBehavior>(DropBehaviorOffset);
	}

	bool ShouldDropOnDeath()
	{
		static auto bDropOnDeathOffset = GetOffset("bDropOnDeath");
		static auto bDropOnDeathFieldMask = GetFieldMask(GetProperty("bDropOnDeath"));
		return ReadBitfieldValue(bDropOnDeathOffset, bDropOnDeathFieldMask);
	}

	bool ShouldIgnoreRespawningOnDrop()
	{
		static auto bIgnoreRespawningForDroppingAsPickupOffset = GetOffset("bIgnoreRespawningForDroppingAsPickup", false);

		if (bIgnoreRespawningForDroppingAsPickupOffset == -1)
			return false;

		static auto bIgnoreRespawningForDroppingAsPickupFieldMask = GetFieldMask(GetProperty("bIgnoreRespawningForDroppingAsPickup"));
		return ReadBitfieldValue(bIgnoreRespawningForDroppingAsPickupOffset, bIgnoreRespawningForDroppingAsPickupFieldMask);
	}

	bool ShouldPersistWhenFinalStackEmpty()
	{
		static auto bPersistInInventoryWhenFinalStackEmptyOffset = GetOffset("bPersistInInventoryWhenFinalStackEmpty", false);

		if (bPersistInInventoryWhenFinalStackEmptyOffset == -1)
			return false;

		static auto bPersistInInventoryWhenFinalStackEmptyFieldMask = GetFieldMask(GetProperty("bPersistInInventoryWhenFinalStackEmpty"));
		return ReadBitfieldValue(bPersistInInventoryWhenFinalStackEmptyOffset, bPersistInInventoryWhenFinalStackEmptyFieldMask);
	}

	bool ShouldFocusWhenAdded()
	{
		static auto bForceFocusWhenAddedOffset = GetOffset("bForceFocusWhenAdded", false);
		
		if (bForceFocusWhenAddedOffset == -1)
			return false;

		static auto bForceFocusWhenAddedFieldMask = GetFieldMask(GetProperty("bForceFocusWhenAdded"));
		return ReadBitfieldValue(bForceFocusWhenAddedOffset, bForceFocusWhenAddedFieldMask);
	}

	bool ShouldForceFocusWhenAdded()
	{
		static auto bForceFocusWhenAddedOffset = GetOffset("bForceFocusWhenAdded");
		
		if (bForceFocusWhenAddedOffset == -1)
			return false;

		static auto bForceFocusWhenAddedFieldMask = GetFieldMask(GetProperty("bForceFocusWhenAdded"));
		return ReadBitfieldValue(bForceFocusWhenAddedOffset, bForceFocusWhenAddedFieldMask);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortWorldItemDefinition");
		return Class;
	}
};
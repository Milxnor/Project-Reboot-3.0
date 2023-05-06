#pragma once

#include "FortItemDefinition.h"

enum class EWorldItemDropBehavior : uint8_t
{
	DropAsPickup = 0,
	DestroyOnDrop = 1,
	DropAsPickupDestroyOnEmpty = 2,
	EWorldItemDropBehavior_MAX = 3
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
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortWorldItemDefinition");
		return Class;
	}
};
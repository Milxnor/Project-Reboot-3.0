#pragma once

#include "FortItemDefinition.h"

class UFortWorldItemDefinition : public UFortItemDefinition
{
public:
	bool CanBeDropped()
	{
		static auto bCanBeDroppedOffset = GetOffset("bCanBeDropped");
		static auto bCanBeDroppedFieldMask = GetFieldMask(GetProperty("bCanBeDropped"));
		return ReadBitfieldValue(bCanBeDroppedOffset, bCanBeDroppedFieldMask);
	}

	bool ShouldDropOnDeath()
	{
		static auto bDropOnDeathOffset = GetOffset("bDropOnDeath");
		static auto bDropOnDeathFieldMask = GetFieldMask(GetProperty("bDropOnDeath"));
		return ReadBitfieldValue(bDropOnDeathOffset, bDropOnDeathFieldMask);
	}

	bool ShouldPersistWhenFinalStackEmpty()
	{
		static auto bPersistInInventoryWhenFinalStackEmptyOffset = GetOffset("bPersistInInventoryWhenFinalStackEmpty", false);

		if (bPersistInInventoryWhenFinalStackEmptyOffset == -1)
			return false;

		static auto bPersistInInventoryWhenFinalStackEmptyFieldMask = GetFieldMask(GetProperty("bPersistInInventoryWhenFinalStackEmpty"));
		return ReadBitfieldValue(bPersistInInventoryWhenFinalStackEmptyOffset, bPersistInInventoryWhenFinalStackEmptyFieldMask);
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
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

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortWorldItemDefinition");
		return Class;
	}
};
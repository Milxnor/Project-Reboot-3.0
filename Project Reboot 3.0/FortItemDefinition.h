#pragma once

#include "FortItem.h"
#include "Object.h"
#include "Class.h"

#include "reboot.h"

class UFortItemDefinition : public UObject
{
public:
	UFortItem* CreateTemporaryItemInstanceBP(int Count, int Level = 1);
	float GetMaxStackSize();

	bool DoesAllowMultipleStacks()
	{
		static auto bAllowMultipleStacksOffset = GetOffset("bAllowMultipleStacks");
		static auto bAllowMultipleStacksFieldMask = GetFieldMask(GetProperty("bAllowMultipleStacks"));
		return ReadBitfieldValue(bAllowMultipleStacksOffset, bAllowMultipleStacksFieldMask);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortItemDefinition");
		return Class;
	}
};
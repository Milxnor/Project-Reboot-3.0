#pragma once

#include "FortAthenaMutator_InventoryOverride.h"
#include "GameplayAbilityTypes.h"
#include "CurveTable.h"
#include "FortItemDefinition.h"
#include "FortInventory.h"

struct FHotfixableInventoryOverrideItem
{
public:
	FScalableFloat                        Count;                                             // 0x0(0x20)(Edit, BlueprintVisible, NativeAccessSpecifierPublic)
	UFortItemDefinition* Item;                                              // 0x20(0x8)(Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

struct FItemLoadoutContainer
{
public:
	FScalableFloat                        bEnabled;                                          // 0x0(0x20)(Edit, DisableEditOnInstance, NativeAccessSpecifierPublic)
	TArray<FItemAndCount>                 Loadout;                                           // 0x20(0x10)(ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	TArray<FHotfixableInventoryOverrideItem> LoadoutList;                                       // 0x30(0x10)(Edit, BlueprintVisible, ZeroConstructor, DisableEditOnInstance, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

struct FItemLoadoutBucket
{
public:
	FScalableFloat                        bEnabled;                                          // 0x0(0x20)(Edit, DisableEditOnInstance, NativeAccessSpecifierPublic)
	TArray<FItemLoadoutContainer>         Loadouts;                                          // 0x20(0x10)(Edit, ZeroConstructor, DisableEditOnInstance, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	uint8                                        Pad_3D83[0x8];                                     // Fixing Size Of Struct [ Dumper-7 ]
};

class AFortAthenaMutator_InventoryOverride_Bucket : public AFortAthenaMutator_InventoryOverride
{
public:
};
#include "FortKismetLibrary.h"
#include "ScriptInterface.h"
#include "FortPickup.h"
#include "FortLootPackage.h"
#include "AbilitySystemComponent.h"

UFortResourceItemDefinition* UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType ResourceType)
{
	if (ResourceType == EFortResourceType::Wood)
	{
		static auto WoodItemData = FindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
		return WoodItemData;
	}
	else if (ResourceType == EFortResourceType::Stone)
	{
		static auto StoneItemData = FindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
		return StoneItemData;
	}
	else if (ResourceType == EFortResourceType::Metal)
	{
		static auto MetalItemData = FindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");
		return MetalItemData;
	}

	return nullptr;

	// The function below doesn't exist on some very old builds.
	
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_GetResourceItemDefinition");

	struct { EFortResourceType type; UFortResourceItemDefinition* ret; } params{ResourceType};
	
	static auto DefaultClass = StaticClass();
	DefaultClass->ProcessEvent(fn, &params);
	return params.ret;
}

void UFortKismetLibrary::ApplyCharacterCosmetics(UObject* WorldContextObject, const TArray<UObject*>& CharacterParts, UObject* PlayerState, bool* bSuccess)
{
	static auto fn = FindObject<UFunction>("/Script/FortniteGame.FortKismetLibrary.ApplyCharacterCosmetics");

	if (fn)
	{
		struct
		{
			UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			TArray<UObject*>                CharacterParts;                                           // (Parm, ZeroConstructor, NativeAccessSpecifierPublic)
			UObject* PlayerState;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			bool                                               bSuccess;                                                 // (Parm, OutParm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		} UFortKismetLibrary_ApplyCharacterCosmetics_Params{ WorldContextObject, CharacterParts, PlayerState };

		static auto DefaultClass = StaticClass();
		DefaultClass->ProcessEvent(fn, &UFortKismetLibrary_ApplyCharacterCosmetics_Params);

		if (bSuccess)
			*bSuccess = UFortKismetLibrary_ApplyCharacterCosmetics_Params.bSuccess;

		return;
	}

	static auto CharacterPartsOffset = PlayerState->GetOffset("CharacterParts", false);

	if (CharacterPartsOffset != -1)
	{
		auto CharacterPartsPS = PlayerState->GetPtr<__int64>("CharacterParts");

		static auto CustomCharacterPartsStruct = FindObject<UStruct>("/Script/FortniteGame.CustomCharacterParts");

		// if (CustomCharacterPartsStruct)
		{
			static auto PartsOffset = FindOffsetStruct("/Script/FortniteGame.CustomCharacterParts", "Parts");
			auto Parts = (UObject**)(__int64(CharacterPartsPS) + PartsOffset); // UCustomCharacterPart* Parts[0x6]

			for (int i = 0; i < CharacterParts.Num(); i++)
			{
				Parts[i] = CharacterParts.at(i);
			}

			static auto OnRep_CharacterPartsFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerState.OnRep_CharacterParts");
			PlayerState->ProcessEvent(OnRep_CharacterPartsFn);
		}
	}
	else
	{
		// TODO Add character data support
	}
}

void UFortKismetLibrary::PickLootDropsWithNamedWeightsHook(UObject* Context, FFrame& Stack, void* Ret)
{
	LOG_INFO(LogDev, __FUNCTION__);
	return PickLootDropsWithNamedWeightsOriginal(Context, Stack, Ret);
}

void UFortKismetLibrary::SpawnItemVariantPickupInWorldHook(UObject* Context, FFrame& Stack, void* Ret)
{
	UObject* WorldContextObject;                                // 0x0(0x8)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	PadHexB0 Params; // = *Alloc<FSpawnItemVariantParams>(FSpawnItemVariantParams::GetStructSize());

	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&Params);

	LOG_INFO(LogDev, __FUNCTION__);

	auto ParamsPtr = (FSpawnItemVariantParams*)&Params;

	auto ItemDefinition = ParamsPtr->GetWorldItemDefinition();

	LOG_INFO(LogDev, "ItemDefinition: {}", ItemDefinition ? ItemDefinition->GetFullName() : "InvalidObject");

	if (!ItemDefinition)
		return SpawnItemVariantPickupInWorldOriginal(Context, Stack, Ret);

	auto& Position = ParamsPtr->GetPosition();

	LOG_INFO(LogDev, "{} {} {}", Position.X, Position.Y, Position.Z);

	auto Pickup = AFortPickup::SpawnPickup(ItemDefinition, Position, ParamsPtr->GetNumberToSpawn(), ParamsPtr->GetSourceType(), ParamsPtr->GetSource());

	return SpawnItemVariantPickupInWorldOriginal(Context, Stack, Ret);
}

bool UFortKismetLibrary::SpawnInstancedPickupInWorldHook(UObject* Context, FFrame& Stack, bool* Ret)
{
	UObject* WorldContextObject;                                // 0x0(0x8)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	UFortWorldItemDefinition* ItemDefinition;                                    // 0x8(0x8)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int32                                        NumberToSpawn;                                     // 0x10(0x4)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                               Position;                                          // 0x14(0xC)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                               Direction;                                         // 0x20(0xC)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int32                                        OverrideMaxStackCount;                             // 0x2C(0x4)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                         bToss;                                             // 0x30(0x1)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                         bRandomRotation;                                   // 0x31(0x1)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                         bBlockedFromAutoPickup;

	LOG_INFO(LogDev, __FUNCTION__);

	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&NumberToSpawn);
	Stack.StepCompiledIn(&Position);
	Stack.StepCompiledIn(&Direction);
	Stack.StepCompiledIn(&OverrideMaxStackCount);
	Stack.StepCompiledIn(&bToss);
	Stack.StepCompiledIn(&bRandomRotation);
	Stack.StepCompiledIn(&bBlockedFromAutoPickup);

	auto Pickup = AFortPickup::SpawnPickup(ItemDefinition, Position, NumberToSpawn, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset, -1, nullptr, nullptr, bToss);

	*Ret = Pickup;
	return *Ret;
}

void UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTierHook(UObject* Context, FFrame& Stack, void* Ret)
{
	LOG_INFO(LogDev, __FUNCTION__);

	return K2_SpawnPickupInWorldWithLootTierOriginal(Context, Stack, Ret);
}

void TestFunctionHook(UObject* Context, FFrame& Stack, void* Ret)
{
	auto PlayerController = (AFortPlayerController*)Context;
	AFortPawn* Pawn = nullptr;

	Stack.StepCompiledIn(&Pawn);
}

void UFortKismetLibrary::CreateTossAmmoPickupForWeaponItemDefinitionAtLocationHook(UObject* Context, FFrame& Stack, void* Ret)
{
	UObject* WorldContextObject; 
	UFortWeaponItemDefinition* WeaponItemDefinition; 
	FGameplayTagContainer                 SourceTags; 
	FVector                               Location; 
	EFortPickupSourceTypeFlag         SourceTypeFlag; 
	EFortPickupSpawnSource            SpawnSource;

	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&WeaponItemDefinition);
	Stack.StepCompiledIn(&SourceTags);
	Stack.StepCompiledIn(&Location);
	Stack.StepCompiledIn(&SourceTypeFlag);
	Stack.StepCompiledIn(&SpawnSource);

	LOG_INFO(LogDev, __FUNCTION__);

	// return CreateTossAmmoPickupForWeaponItemDefinitionAtLocationOriginal(Context, Stack, Ret);

	int Count = 1; // uh?

	auto AmmoDefinition = WeaponItemDefinition->GetAmmoData();

	if (!AmmoDefinition)
		return CreateTossAmmoPickupForWeaponItemDefinitionAtLocationOriginal(Context, Stack, Ret);

	auto AmmoPickup = AFortPickup::SpawnPickup(AmmoDefinition, Location, Count, SourceTypeFlag, SpawnSource);

	return CreateTossAmmoPickupForWeaponItemDefinitionAtLocationOriginal(Context, Stack, Ret);
}

void UFortKismetLibrary::GiveItemToInventoryOwnerHook(UObject* Context, FFrame& Stack, void* Ret)
{
	static auto ItemLevelOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "ItemLevel", false);
	static auto PickupInstigatorHandleOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "PickupInstigatorHandle", false);
	static auto ItemVariantGuidOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "ItemVariantGuid", false);

	TScriptInterface<UFortInventoryOwnerInterface> InventoryOwner; // = *(TScriptInterface<UFortInventoryOwnerInterface>*)(__int64(Params) + InventoryOwnerOffset);
	UFortWorldItemDefinition* ItemDefinition = nullptr; // *(UFortWorldItemDefinition**)(__int64(Params) + ItemDefinitionOffset);
	FGuid ItemVariantGuid;
	int NumberToGive; // = *(int*)(__int64(Params) + NumberToGiveOffset);
	bool bNotifyPlayer; // = *(bool*)(__int64(Params) + bNotifyPlayerOffset);
	int ItemLevel; // = *(int*)(__int64(Params) + ItemLevelOffset);
	int PickupInstigatorHandle; // = *(int*)(__int64(Params) + PickupInstigatorHandleOffset);

	Stack.StepCompiledIn(&InventoryOwner);
	Stack.StepCompiledIn(&ItemDefinition);
	if (ItemVariantGuidOffset != -1) Stack.StepCompiledIn(&ItemVariantGuid);
	Stack.StepCompiledIn(&NumberToGive);
	Stack.StepCompiledIn(&bNotifyPlayer);

	LOG_INFO(LogDev, __FUNCTION__);

	if (ItemLevelOffset != -1)
		Stack.StepCompiledIn(&ItemLevel);

	if (PickupInstigatorHandleOffset != -1)
		Stack.StepCompiledIn(&PickupInstigatorHandle);

	if (!ItemDefinition)
		return GiveItemToInventoryOwnerOriginal(Context, Stack, Ret);

	auto InterfacePointer = InventoryOwner.InterfacePointer;

	LOG_INFO(LogDev, "InterfacePointer: {}", __int64(InterfacePointer));

	if (!InterfacePointer)
		return GiveItemToInventoryOwnerOriginal(Context, Stack, Ret);

	auto ObjectPointer = InventoryOwner.ObjectPointer;

	LOG_INFO(LogDev, "ObjectPointer: {}", __int64(ObjectPointer));

	if (!ObjectPointer)
		return GiveItemToInventoryOwnerOriginal(Context, Stack, Ret);

	LOG_INFO(LogDev, "ObjectPointer Name: {}", ObjectPointer->GetFullName());

	auto PlayerController = Cast<AFortPlayerController>(ObjectPointer);

	if (!PlayerController)
		return GiveItemToInventoryOwnerOriginal(Context, Stack, Ret);

	bool bShouldUpdate = false;
	LOG_INFO(LogDev, "ItemDefinition: {}", __int64(ItemDefinition));
	LOG_INFO(LogDev, "ItemDefinition Name: {}", ItemDefinition->GetFullName());
	PlayerController->GetWorldInventory()->AddItem(ItemDefinition, &bShouldUpdate, NumberToGive, -1, bNotifyPlayer);

	if (bShouldUpdate)
		PlayerController->GetWorldInventory()->Update();

	return GiveItemToInventoryOwnerOriginal(Context, Stack, Ret);
}

void UFortKismetLibrary::K2_RemoveItemFromPlayerHook(UObject* Context, FFrame& Stack, void* Ret)
{
	static auto PlayerControllerOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayer", "PlayerController");
	static auto ItemDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayer", "ItemDefinition");
	static auto AmountToRemoveOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayer", "AmountToRemove");

	AFortPlayerController* PlayerController = nullptr;
	UFortWorldItemDefinition* ItemDefinition = nullptr;
	int AmountToRemove;

	Stack.StepCompiledIn(&PlayerController);
	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&AmountToRemove);

	LOG_INFO(LogDev, __FUNCTION__);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return K2_RemoveItemFromPlayerOriginal(Context, Stack, Ret);

	auto ItemInstance = WorldInventory->FindItemInstance(ItemDefinition);

	if (!ItemInstance)
		return K2_RemoveItemFromPlayerOriginal(Context, Stack, Ret);

	bool bShouldUpdate = false;
	WorldInventory->RemoveItem(ItemInstance->GetItemEntry()->GetItemGuid(), &bShouldUpdate, AmountToRemove);

	if (bShouldUpdate)
		WorldInventory->Update();

	LOG_INFO(LogDev, "Removed {}!", AmountToRemove);

	return K2_RemoveItemFromPlayerOriginal(Context, Stack, Ret);
}

void UFortKismetLibrary::K2_RemoveItemFromPlayerByGuidHook(UObject* Context, FFrame& Stack, void* Ret)
{
	AFortPlayerController* PlayerController = nullptr;                                         // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FGuid                                       ItemGuid;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                AmountToRemove;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bForceRemoval;

	Stack.StepCompiledIn(&PlayerController);
	Stack.StepCompiledIn(&ItemGuid);
	Stack.StepCompiledIn(&AmountToRemove);
	Stack.StepCompiledIn(&bForceRemoval);

	LOG_INFO(LogDev, __FUNCTION__);

	if (!PlayerController)
		return;

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return K2_RemoveItemFromPlayerByGuidOriginal(Context, Stack, Ret);

	bool bShouldUpdate = false;
	WorldInventory->RemoveItem(ItemGuid, &bShouldUpdate, AmountToRemove, bForceRemoval);

	if (bShouldUpdate)
		WorldInventory->Update();

	return K2_RemoveItemFromPlayerByGuidOriginal(Context, Stack, Ret);
}

void UFortKismetLibrary::K2_GiveItemToPlayerHook(UObject* Context, FFrame& Stack, void* Ret)
{
	static auto ItemVariantGuidOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.K2_GiveItemToPlayer", "ItemVariantGuid", false);

	auto Params = Stack.Locals;

	AFortPlayerController* PlayerController = nullptr;
	UFortWorldItemDefinition* ItemDefinition = nullptr;
	FGuid ItemVariantGuid;
	int NumberToGive;
	bool bNotifyPlayer;

	Stack.StepCompiledIn(&PlayerController);
	Stack.StepCompiledIn(&ItemDefinition);
	if (ItemVariantGuidOffset != -1) Stack.StepCompiledIn(&ItemVariantGuid);
	Stack.StepCompiledIn(&NumberToGive);
	Stack.StepCompiledIn(&bNotifyPlayer);

	LOG_INFO(LogDev, __FUNCTION__);

	if (!PlayerController || !ItemDefinition)
		return K2_GiveItemToPlayerOriginal(Context, Stack, Ret);

	bool bShouldUpdate = false;
	PlayerController->GetWorldInventory()->AddItem(ItemDefinition, &bShouldUpdate, NumberToGive, -1, bNotifyPlayer);
	
	if (bShouldUpdate)
		PlayerController->GetWorldInventory()->Update();

	return K2_GiveItemToPlayerOriginal(Context, Stack, Ret);
}

void UFortKismetLibrary::K2_GiveBuildingResourceHook(UObject* Context, FFrame& Stack, void* Ret)
{
	LOG_INFO(LogDev, "K2_GiveBuildingResourceHook!");

	AFortPlayerController* Controller;
	EFortResourceType ResourceType;
	int ResourceAmount;

	Stack.StepCompiledIn(&Controller);
	Stack.StepCompiledIn(&ResourceType);
	Stack.StepCompiledIn(&ResourceAmount);

	if (!Controller)
		return K2_GiveBuildingResourceOriginal(Context, Stack, Ret);

	auto WorldInventory = Controller->GetWorldInventory();

	if (!WorldInventory)
		return K2_GiveBuildingResourceOriginal(Context, Stack, Ret);

	auto ItemDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(ResourceType);

	if (!ItemDefinition)
		return K2_GiveBuildingResourceOriginal(Context, Stack, Ret);

	bool bShouldUpdate = false;
	WorldInventory->AddItem(ItemDefinition, &bShouldUpdate, ResourceAmount, 0);

	if (bShouldUpdate)
		WorldInventory->Update();

	return K2_GiveBuildingResourceOriginal(Context, Stack, Ret);
}

int UFortKismetLibrary::K2_RemoveFortItemFromPlayerHook1(AFortPlayerController* PlayerController, UFortItem* Item, int AmountToRemove, bool bForceRemoval)
{
	LOG_INFO(LogDev, "K2_RemoveFortItemFromPlayerHookNative!");

	if (!PlayerController || !Item)
		return 0;

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return 0;

	bool bShouldUpdate = false;
	WorldInventory->RemoveItem(Item->GetItemEntry()->GetItemGuid(), &bShouldUpdate, AmountToRemove, bForceRemoval);

	if (bShouldUpdate)
		WorldInventory->Update();

	return 1; // idk probably how much we removed
}

void UFortKismetLibrary::K2_RemoveFortItemFromPlayerHook(UObject* Context, FFrame& Stack, void* Ret)
{
	AFortPlayerController* PlayerController = nullptr;                                         // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	UFortItem* Item = nullptr;                                                     // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                AmountToRemove;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bForceRemoval;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	Stack.StepCompiledIn(&PlayerController);
	Stack.StepCompiledIn(&Item);
	Stack.StepCompiledIn(&AmountToRemove);
	Stack.StepCompiledIn(&bForceRemoval);

	LOG_INFO(LogDev, __FUNCTION__);

	if (!PlayerController || !Item)
		return;

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return K2_RemoveFortItemFromPlayerOriginal(Context, Stack, Ret);

	LOG_INFO(LogDev, "bForceRemoval: {}", bForceRemoval);
	bool bShouldUpdate = false;
	WorldInventory->RemoveItem(Item->GetItemEntry()->GetItemGuid(), &bShouldUpdate, AmountToRemove, bForceRemoval);

	if (bShouldUpdate)
		WorldInventory->Update();

	return K2_RemoveFortItemFromPlayerOriginal(Context, Stack, Ret);
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorldWithClassHook(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
	UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	UFortWorldItemDefinition* ItemDefinition;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	UClass* PickupClass;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                NumberToSpawn;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     Position;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     Direction;                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                OverrideMaxStackCount;                                    // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bToss;                                                    // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bRandomRotation;                                          // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bBlockedFromAutoPickup;                                   // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                PickupInstigatorHandle;                                   // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	EFortPickupSourceTypeFlag                          SourceType;                                               // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	EFortPickupSpawnSource                             Source;                                                   // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	AFortPlayerController* OptionalOwnerPC;                                          // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bPickupOnlyRelevantToOwner;                               // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&PickupClass);
	Stack.StepCompiledIn(&NumberToSpawn);
	Stack.StepCompiledIn(&Position);
	Stack.StepCompiledIn(&Direction);
	Stack.StepCompiledIn(&OverrideMaxStackCount);
	Stack.StepCompiledIn(&bToss);
	Stack.StepCompiledIn(&bRandomRotation);
	Stack.StepCompiledIn(&bBlockedFromAutoPickup);
	Stack.StepCompiledIn(&PickupInstigatorHandle);
	Stack.StepCompiledIn(&SourceType);
	Stack.StepCompiledIn(&Source);
	Stack.StepCompiledIn(&OptionalOwnerPC);
	Stack.StepCompiledIn(&bPickupOnlyRelevantToOwner);

	if (!ItemDefinition)
		return K2_SpawnPickupInWorldWithClassOriginal(Context, Stack, Ret);

	LOG_INFO(LogDev, "PickupClass: {}", PickupClass ? PickupClass->GetFullName() : "InvalidObject");

	LOG_INFO(LogDev, __FUNCTION__);

	auto aa = AFortPickup::SpawnPickup(ItemDefinition, Position, NumberToSpawn, SourceType, Source, -1, nullptr, PickupClass, bToss);

	K2_SpawnPickupInWorldWithClassOriginal(Context, Stack, Ret);

	*Ret = aa;
	return *Ret;
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorldHook(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
	UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	UFortWorldItemDefinition* ItemDefinition;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                NumberToSpawn;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     Position;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     Direction;                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                OverrideMaxStackCount;                                    // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bToss;                                                    // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bRandomRotation;                                          // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bBlockedFromAutoPickup;                                   // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                PickupInstigatorHandle;                                   // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	EFortPickupSourceTypeFlag                          SourceType;                                               // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	EFortPickupSpawnSource                             Source;                                                   // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	AFortPlayerController* OptionalOwnerPC;                                          // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool                                               bPickupOnlyRelevantToOwner;                               // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&NumberToSpawn);
	Stack.StepCompiledIn(&Position);
	Stack.StepCompiledIn(&Direction);
	Stack.StepCompiledIn(&OverrideMaxStackCount);
	Stack.StepCompiledIn(&bToss);
	Stack.StepCompiledIn(&bRandomRotation);
	Stack.StepCompiledIn(&bBlockedFromAutoPickup);
	Stack.StepCompiledIn(&PickupInstigatorHandle);
	Stack.StepCompiledIn(&SourceType);
	Stack.StepCompiledIn(&Source);
	Stack.StepCompiledIn(&OptionalOwnerPC);
	Stack.StepCompiledIn(&bPickupOnlyRelevantToOwner);

	LOG_INFO(LogDev, "[{}] ItemDefinition: {}", __FUNCTION__, ItemDefinition ? ItemDefinition->GetFullName() : "");

	if (!ItemDefinition)
		return	K2_SpawnPickupInWorldOriginal(Context, Stack, Ret);

	auto aa = AFortPickup::SpawnPickup(ItemDefinition, Position, NumberToSpawn, SourceType, Source, -1, nullptr, nullptr, bToss);

	K2_SpawnPickupInWorldOriginal(Context, Stack, Ret);

	*Ret = aa;
	return *Ret;
}

bool UFortKismetLibrary::PickLootDropsHook(UObject* Context, FFrame& Stack, bool* Ret)
{
	static auto WorldContextObjectOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.PickLootDrops", "WorldContextObject", false);

	UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	TArray<FFortItemEntry>                      OutLootToDropTempBuf;                                            // (Parm, OutParm, ZeroConstructor, NativeAccessSpecifierPublic)
	FName                                       TierGroupName;                                            // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                WorldLevel;                                               // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                ForcedLootTier;                                           // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	if (WorldContextObjectOffset != -1)
		Stack.StepCompiledIn(&WorldContextObject);

	auto& OutLootToDrop = Stack.StepCompiledInRef<TArray<FFortItemEntry>>(&OutLootToDropTempBuf);
	Stack.StepCompiledIn(&TierGroupName);
	Stack.StepCompiledIn(&WorldLevel);
	Stack.StepCompiledIn(&ForcedLootTier);

	LOG_INFO(LogDev, "Picking loot for {}.", TierGroupName.ComparisonIndex.Value ? TierGroupName.ToString() : "InvalidName");

	auto LootDrops = PickLootDrops(TierGroupName, true);

	/* LootDrop skuffed{};
	skuffed.ItemDefinition = FindObject<UFortItemDefinition>("AGID_CarminePack", nullptr, ANY_PACKAGE);
	skuffed.Count = 1;

	LootDrops.push_back(skuffed); */

	for (int i = 0; i < LootDrops.size(); i++)
	{
		auto& LootDrop = LootDrops.at(i);

		auto NewEntry = FFortItemEntry::MakeItemEntry(LootDrop.ItemDefinition, LootDrop.Count, LootDrop.LoadedAmmo);

		OutLootToDrop.AddPtr(NewEntry, FFortItemEntry::GetStructSize());
	}

	PickLootDropsOriginal(Context, Stack, Ret);

	*Ret = true;
	return *Ret;
}

UClass* UFortKismetLibrary::StaticClass()
{
	static auto ptr = FindObject<UClass>(L"/Script/FortniteGame.FortKismetLibrary");
	return ptr;
}
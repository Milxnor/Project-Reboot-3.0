#include "FortKismetLibrary.h"
#include "ScriptInterface.h"
#include "FortPickup.h"
#include "FortLootPackage.h"

UFortResourceItemDefinition* UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType ResourceType)
{
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

void UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTierHook(UObject* Context, FFrame& Stack, void* Ret)
{
	LOG_INFO(LogDev, __FUNCTION__);

	return K2_SpawnPickupInWorldWithLootTierOriginal(Context, Stack, Ret);
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

	return CreateTossAmmoPickupForWeaponItemDefinitionAtLocationOriginal(Context, Stack, Ret);

	int Count = 1;

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

	TScriptInterface<UFortInventoryOwnerInterface> InventoryOwner; // = *(TScriptInterface<UFortInventoryOwnerInterface>*)(__int64(Params) + InventoryOwnerOffset);
	UFortWorldItemDefinition* ItemDefinition = nullptr; // *(UFortWorldItemDefinition**)(__int64(Params) + ItemDefinitionOffset);
	int NumberToGive; // = *(int*)(__int64(Params) + NumberToGiveOffset);
	bool bNotifyPlayer; // = *(bool*)(__int64(Params) + bNotifyPlayerOffset);
	int ItemLevel; // = *(int*)(__int64(Params) + ItemLevelOffset);
	int PickupInstigatorHandle; // = *(int*)(__int64(Params) + PickupInstigatorHandleOffset);

	Stack.StepCompiledIn(&InventoryOwner);
	Stack.StepCompiledIn(&ItemDefinition);
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
	WorldInventory->RemoveItem(ItemGuid, &bShouldUpdate, AmountToRemove);

	if (bShouldUpdate)
		WorldInventory->Update();

	return K2_RemoveItemFromPlayerByGuidOriginal(Context, Stack, Ret);
}

void UFortKismetLibrary::K2_GiveItemToPlayerHook(UObject* Context, FFrame& Stack, void* Ret)
{
	auto Params = Stack.Locals;

	AFortPlayerController* PlayerController = nullptr;
	UFortWorldItemDefinition* ItemDefinition = nullptr;
	int NumberToGive;
	bool bNotifyPlayer;

	Stack.StepCompiledIn(&PlayerController);
	Stack.StepCompiledIn(&ItemDefinition);
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

	if (!PlayerController)
		return;

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return K2_RemoveFortItemFromPlayerOriginal(Context, Stack, Ret);

	bool bShouldUpdate = false;
	WorldInventory->RemoveItem(Item->GetItemEntry()->GetItemGuid(), &bShouldUpdate, AmountToRemove);

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

	auto aa = AFortPickup::SpawnPickup(ItemDefinition, Position, NumberToSpawn, SourceType, Source, -1, nullptr, PickupClass);

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

	auto aa = AFortPickup::SpawnPickup(ItemDefinition, Position, NumberToSpawn, SourceType, Source);

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
		auto NewEntry = Alloc<FFortItemEntry>(FFortItemEntry::GetStructSize());
		auto& LootDrop = LootDrops.at(i);

		NewEntry->GetItemDefinition() = LootDrop.ItemDefinition;
		NewEntry->GetCount() = LootDrop.Count;
		NewEntry->GetLoadedAmmo() = LootDrop.LoadedAmmo;

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
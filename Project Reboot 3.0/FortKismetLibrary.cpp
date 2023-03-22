#include "FortKismetLibrary.h"
#include "ScriptInterface.h"
#include "FortPickup.h"

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

	if (CharacterPartsOffset != 0)
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

void UFortKismetLibrary::GiveItemToInventoryOwnerHook(UObject* Context, FFrame& Stack, void* Ret)
{
	auto Params = /*(UFortKismetLibrary_GiveItemToInventoryOwner_Params*)*/Stack.Locals;

	static auto InventoryOwnerOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "InventoryOwner");
	static auto ItemDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "ItemDefinition");
	static auto NumberToGiveOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "NumberToGive");
	static auto bNotifyPlayerOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "bNotifyPlayer");
	static auto ItemLevelOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "ItemLevel", false);
	static auto PickupInstigatorHandleOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "PickupInstigatorHandle", false);

	// return GiveItemToInventoryOwnerOriginal(Context, Stack, Ret);

	TScriptInterface<UFortInventoryOwnerInterface> InventoryOwner; // = *(TScriptInterface<UFortInventoryOwnerInterface>*)(__int64(Params) + InventoryOwnerOffset);
	UFortWorldItemDefinition* ItemDefinition = nullptr; // *(UFortWorldItemDefinition**)(__int64(Params) + ItemDefinitionOffset);
	int NumberToGive; // = *(int*)(__int64(Params) + NumberToGiveOffset);
	bool bNotifyPlayer; // = *(bool*)(__int64(Params) + bNotifyPlayerOffset);
	int ItemLevel; // = *(int*)(__int64(Params) + ItemLevelOffset);
	int PickupInstigatorHandle; // = *(int*)(__int64(Params) + PickupInstigatorHandleOffset);

	Stack.Step(Stack.Object, &InventoryOwner);
	Stack.Step(Stack.Object, &ItemDefinition);
	Stack.Step(Stack.Object, &NumberToGive);
	Stack.Step(Stack.Object, &bNotifyPlayer);

	if (ItemLevelOffset != 0)
		Stack.Step(Stack.Object, &ItemLevel);

	if (PickupInstigatorHandleOffset != 0)
		Stack.Step(Stack.Object, &PickupInstigatorHandle);

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

	Stack.Step(Stack.Object, &PlayerController);
	Stack.Step(Stack.Object, &ItemDefinition);
	Stack.Step(Stack.Object, &AmountToRemove);

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

	Stack.Step(Stack.Object, &PlayerController);
	Stack.Step(Stack.Object, &ItemGuid);
	Stack.Step(Stack.Object, &AmountToRemove);
	Stack.Step(Stack.Object, &bForceRemoval);

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

	Stack.Step(Stack.Object, &PlayerController);
	Stack.Step(Stack.Object, &ItemDefinition);
	Stack.Step(Stack.Object, &NumberToGive);
	Stack.Step(Stack.Object, &bNotifyPlayer);

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

	Stack.Step(Stack.Object, &PlayerController);
	Stack.Step(Stack.Object, &Item);
	Stack.Step(Stack.Object, &AmountToRemove);
	Stack.Step(Stack.Object, &bForceRemoval);

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


	Stack.Step(Stack.Object, &WorldContextObject);
	Stack.Step(Stack.Object, &ItemDefinition);
	Stack.Step(Stack.Object, &PickupClass);
	Stack.Step(Stack.Object, &NumberToSpawn);
	Stack.Step(Stack.Object, &Position);
	Stack.Step(Stack.Object, &Direction);
	Stack.Step(Stack.Object, &OverrideMaxStackCount);
	Stack.Step(Stack.Object, &bToss);
	Stack.Step(Stack.Object, &bRandomRotation);
	Stack.Step(Stack.Object, &bBlockedFromAutoPickup);
	Stack.Step(Stack.Object, &PickupInstigatorHandle);
	Stack.Step(Stack.Object, &SourceType);
	Stack.Step(Stack.Object, &Source);
	Stack.Step(Stack.Object, &OptionalOwnerPC);
	Stack.Step(Stack.Object, &bPickupOnlyRelevantToOwner);

	if (!ItemDefinition)
		return K2_SpawnPickupInWorldWithClassOriginal(Context, Stack, Ret);

	LOG_INFO(LogDev, "PickupClass: {}", PickupClass ? PickupClass->GetFullName() : "InvalidObject")

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

	Stack.Step(Stack.Object, &WorldContextObject);
	Stack.Step(Stack.Object, &ItemDefinition);
	Stack.Step(Stack.Object, &NumberToSpawn);
	Stack.Step(Stack.Object, &Position);
	Stack.Step(Stack.Object, &Direction);
	Stack.Step(Stack.Object, &OverrideMaxStackCount);
	Stack.Step(Stack.Object, &bToss);
	Stack.Step(Stack.Object, &bRandomRotation);
	Stack.Step(Stack.Object, &bBlockedFromAutoPickup);
	Stack.Step(Stack.Object, &PickupInstigatorHandle);
	Stack.Step(Stack.Object, &SourceType);
	Stack.Step(Stack.Object, &Source);
	Stack.Step(Stack.Object, &OptionalOwnerPC);
	Stack.Step(Stack.Object, &bPickupOnlyRelevantToOwner);

	auto aa = AFortPickup::SpawnPickup(ItemDefinition, Position, NumberToSpawn, SourceType, Source);

	K2_SpawnPickupInWorldOriginal(Context, Stack, Ret);

	*Ret = aa;
	return *Ret;
}

bool UFortKismetLibrary::PickLootDropsHook(UObject* Context, FFrame& Stack, bool* Ret)
{
	UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	TArray<__int64>                      OutLootToDrop;                                            // (Parm, OutParm, ZeroConstructor, NativeAccessSpecifierPublic)
	FName                                       TierGroupName;                                            // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                WorldLevel;                                               // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                ForcedLootTier;                                           // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	Stack.Step(Stack.Object, &WorldContextObject);
	Stack.Step(Stack.Object, &OutLootToDrop);
	Stack.Step(Stack.Object, &TierGroupName);
	Stack.Step(Stack.Object, &WorldLevel);
	Stack.Step(Stack.Object, &ForcedLootTier);

	PickLootDropsOriginal(Context, Stack, Ret);

	*Ret = true;
	return *Ret;
}

UClass* UFortKismetLibrary::StaticClass()
{
	static auto ptr = FindObject<UClass>(L"/Script/FortniteGame.FortKismetLibrary");
	return ptr;
}
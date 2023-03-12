#include "FortKismetLibrary.h"
#include "ScriptInterface.h"

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
	static auto ItemLevelOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "ItemLevel");
	static auto PickupInstigatorHandleOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner", "PickupInstigatorHandle");

	LOG_INFO(LogDev, "wtf: {}", __int64(Stack.Code));

	return;

	TScriptInterface<UFortInventoryOwnerInterface> InventoryOwner; // = *(TScriptInterface<UFortInventoryOwnerInterface>*)(__int64(Params) + InventoryOwnerOffset);
	UFortWorldItemDefinition* ItemDefinition; // *(UFortWorldItemDefinition**)(__int64(Params) + ItemDefinitionOffset);
	int NumberToGive; // = *(int*)(__int64(Params) + NumberToGiveOffset);
	bool bNotifyPlayer; // = *(bool*)(__int64(Params) + bNotifyPlayerOffset);
	int ItemLevel; // = *(int*)(__int64(Params) + ItemLevelOffset);
	int PickupInstigatorHandle; // = *(int*)(__int64(Params) + PickupInstigatorHandleOffset);

	Stack.Step(Context, &InventoryOwner);
	Stack.Step(Context, &ItemDefinition);
	Stack.Step(Context, &NumberToGive);
	Stack.Step(Context, &bNotifyPlayer);
	Stack.Step(Context, &ItemLevel);
	Stack.Step(Context, &PickupInstigatorHandle);

	auto InterfacePointer = InventoryOwner.InterfacePointer;

	LOG_INFO(LogDev, "InterfacePointer: {}", __int64(InterfacePointer));

	if (!InterfacePointer)
		return;

	auto ObjectPointer = InventoryOwner.ObjectPointer;

	if (!ObjectPointer)
		return;

	// LOG_INFO(LogDev, "ObjectPointer Name: {}", ObjectPointer->GetFullName());

	return GiveItemToInventoryOwnerOriginal(Context, Stack, Ret);
}

void UFortKismetLibrary::K2_RemoveItemFromPlayerHook(UObject* Context, FFrame& Stack, void* Ret)
{
	struct UFortKismetLibrary_K2_RemoveItemFromPlayer_Params
	{
		AFortPlayerController* PlayerController;                                         // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		UFortWorldItemDefinition* ItemDefinition;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		int                                                AmountToRemove;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		bool                                               bForceRemoval;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		int                                                ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	};

	auto Params = /*(UFortKismetLibrary_K2_RemoveItemFromPlayer_Params*)*/Stack.Locals;

	static auto PlayerControllerOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayer", "PlayerController");
	static auto ItemDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayer", "ItemDefinition");
	static auto AmountToRemoveOffset = FindOffsetStruct("/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayer", "AmountToRemove");

	auto PlayerController = *(AFortPlayerController**)(__int64(Params) + PlayerControllerOffset);
	auto ItemDefinition = *(UFortWorldItemDefinition**)(__int64(Params) + ItemDefinitionOffset);
	auto AmountToRemove = *(int*)(__int64(Params) + AmountToRemoveOffset);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return;

	auto ItemInstance = WorldInventory->FindItemInstance(ItemDefinition);

	if (!ItemInstance)
		return;

	bool bShouldUpdate = false;
	WorldInventory->RemoveItem(ItemInstance->GetItemEntry()->GetItemGuid(), &bShouldUpdate, AmountToRemove);

	if (bShouldUpdate)
		WorldInventory->Update();

	return K2_RemoveItemFromPlayerOriginal(Context, Stack, Ret);
}

void UFortKismetLibrary::K2_GiveItemToPlayerHook(UObject* Context, FFrame& Stack, void* Ret)
{
	struct UFortKismetLibrary_K2_GiveItemToPlayer_Params
	{
		AFortPlayerController* PlayerController;                                         // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		UFortWorldItemDefinition* ItemDefinition;                                           // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		int                                                NumberToGive;                                             // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		bool                                               bNotifyPlayer;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	};

	auto Params = (UFortKismetLibrary_K2_GiveItemToPlayer_Params*)Stack.Locals;

	auto PlayerController = Params->PlayerController;
	auto ItemDefinition = Params->ItemDefinition;
	auto NumberToGive = Params->NumberToGive;
	auto bNotifyPlayer = Params->bNotifyPlayer;

	bool bShouldUpdate = false;
	PlayerController->GetWorldInventory()->AddItem(ItemDefinition, &bShouldUpdate, NumberToGive, -1, bNotifyPlayer);
	
	if (bShouldUpdate)
		PlayerController->GetWorldInventory()->Update();

	return K2_GiveItemToPlayerOriginal(Context, Stack, Ret);
}

UClass* UFortKismetLibrary::StaticClass()
{
	static auto ptr = FindObject<UClass>(L"/Script/FortniteGame.FortKismetLibrary");
	return ptr;
}
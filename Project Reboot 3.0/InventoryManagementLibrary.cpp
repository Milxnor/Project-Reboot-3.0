#include "InventoryManagementLibrary.h"
#include "FortItemDefinition.h"
#include "FortKismetLibrary.h"
#include "ScriptInterface.h"

void UInventoryManagementLibrary::SwapItemsHook(UObject* Context, FFrame& Stack, void* Ret)
{
	TScriptInterface<UFortInventoryOwnerInterface> SourceOwner;
	TScriptInterface<UFortInventoryOwnerInterface> TargetOwner;
	TArray<FItemGuidAndCount> ItemGuids;

	Stack.StepCompiledIn(&SourceOwner);
	Stack.StepCompiledIn(&TargetOwner);
	Stack.StepCompiledIn(&ItemGuids);

	auto SourceObjectPointer = SourceOwner.ObjectPointer;
	auto SourcePlayerController = Cast<AFortPlayerController>(SourceObjectPointer);

	LOG_INFO(LogDev, "Swapping.");

	if (!SourcePlayerController)
		return SwapItemOriginal(Context, Stack, Ret);

	auto SourceWorldInventory = SourcePlayerController->GetWorldInventory();

	if (!SourceWorldInventory)
		return SwapItemsOriginal(Context, Stack, Ret);

	auto TargetObjectPointer = TargetOwner.ObjectPointer;
	auto TargetPlayerController = Cast<AFortPlayerController>(TargetObjectPointer);

	if (!TargetPlayerController)
		return SwapItemsOriginal(Context, Stack, Ret);

	auto TargetWorldInventory = TargetPlayerController->GetWorldInventory();

	if (!TargetWorldInventory)
		return SwapItemsOriginal(Context, Stack, Ret);

	for (int i = 0; i < ItemGuids.size(); i++)
	{
		auto& CurrentItemGuid = ItemGuids.at(i);

		auto ItemToSwapInstance = SourceWorldInventory->FindItemInstance(CurrentItemGuid.ItemGuid);

		if (!ItemToSwapInstance)
			continue;

		TargetWorldInventory->AddItem(ItemToSwapInstance->GetItemEntry()->GetItemDefinition(), nullptr, CurrentItemGuid.Count, ItemToSwapInstance->GetItemEntry()->GetLoadedAmmo());
		SourceWorldInventory->RemoveItem(CurrentItemGuid.ItemGuid, nullptr, CurrentItemGuid.Count); // should we check return value?
	}

	TargetWorldInventory->Update();
	SourceWorldInventory->Update();

	return SwapItemsOriginal(Context, Stack, Ret);
}

void UInventoryManagementLibrary::SwapItemHook(UObject* Context, FFrame& Stack, void* Ret)
{
	TScriptInterface<UFortInventoryOwnerInterface> SourceOwner;
	TScriptInterface<UFortInventoryOwnerInterface> TargetOwner;
	FGuid ItemGuid;
	int32 Count;

	Stack.StepCompiledIn(&SourceOwner);
	Stack.StepCompiledIn(&TargetOwner);
	Stack.StepCompiledIn(&ItemGuid);
	Stack.StepCompiledIn(&Count);

	auto SourceObjectPointer = SourceOwner.ObjectPointer;
	auto SourcePlayerController = Cast<AFortPlayerController>(SourceObjectPointer);

	LOG_INFO(LogDev, "Swapping.");

	if (!SourcePlayerController)
		return SwapItemOriginal(Context, Stack, Ret);

	auto SourceWorldInventory = SourcePlayerController->GetWorldInventory();

	if (!SourceWorldInventory)
		return SwapItemOriginal(Context, Stack, Ret);

	auto TargetObjectPointer = TargetOwner.ObjectPointer;
	auto TargetPlayerController = Cast<AFortPlayerController>(TargetObjectPointer);

	if (!TargetPlayerController)
		return SwapItemOriginal(Context, Stack, Ret);

	auto TargetWorldInventory = TargetPlayerController->GetWorldInventory();

	if (!TargetWorldInventory)
		return SwapItemOriginal(Context, Stack, Ret);

	auto ItemToSwapInstance = SourceWorldInventory->FindItemInstance(ItemGuid);

	if (!ItemToSwapInstance)
		return SwapItemOriginal(Context, Stack, Ret);

	bool bShouldUpdateTarget = false;
	TargetWorldInventory->AddItem(ItemToSwapInstance->GetItemEntry()->GetItemDefinition(), &bShouldUpdateTarget, Count, ItemToSwapInstance->GetItemEntry()->GetLoadedAmmo());

	if (bShouldUpdateTarget)
		TargetWorldInventory->Update();

	bool bShouldUpdateSource = false;
	SourceWorldInventory->RemoveItem(ItemGuid, &bShouldUpdateSource, Count); // should we check return value?

	if (bShouldUpdateSource)
		SourceWorldInventory->Update();

	return SwapItemOriginal(Context, Stack, Ret);
}

void UInventoryManagementLibrary::RemoveItemsHook(UObject* Context, FFrame& Stack, void* Ret)
{
	TScriptInterface<UFortInventoryOwnerInterface> InventoryOwner;
	TArray<FItemGuidAndCount> Items;

	Stack.StepCompiledIn(&InventoryOwner);
	Stack.StepCompiledIn(&Items);

	auto ObjectPointer = InventoryOwner.ObjectPointer;
	auto PlayerController = Cast<AFortPlayerController>(ObjectPointer);

	LOG_INFO(LogDev, "Taking list {} items from {}.", Items.Num(), __int64(PlayerController));

	if (!PlayerController)
		return RemoveItemOriginal(Context, Stack, Ret);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return RemoveItemOriginal(Context, Stack, Ret);

	for (int i = 0; i < Items.Num(); i++)
	{
		WorldInventory->RemoveItem(Items.at(i).ItemGuid, nullptr, Items.at(i).Count);
	}

	WorldInventory->Update();

	return RemoveItemOriginal(Context, Stack, Ret);
}

void UInventoryManagementLibrary::RemoveItemHook(UObject* Context, FFrame& Stack, void* Ret)
{
	TScriptInterface<UFortInventoryOwnerInterface> InventoryOwner;                                    // 0x0(0x10)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, NativeAccessSpecifierPublic)
	FGuid ItemGuid;                                          // 0x10(0x10)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int32 Count;

	Stack.StepCompiledIn(&InventoryOwner);
	Stack.StepCompiledIn(&ItemGuid);
	Stack.StepCompiledIn(&Count);

	auto ObjectPointer = InventoryOwner.ObjectPointer;
	auto PlayerController = Cast<AFortPlayerController>(ObjectPointer);

	LOG_INFO(LogDev, "Taking {} items from {}.", Count, __int64(PlayerController));

	if (!PlayerController)
		return RemoveItemOriginal(Context, Stack, Ret);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return RemoveItemOriginal(Context, Stack, Ret);

	bool bShouldUpdate = false;
	WorldInventory->RemoveItem(ItemGuid, &bShouldUpdate, Count);

	if (bShouldUpdate)
		WorldInventory->Update();

	return RemoveItemOriginal(Context, Stack, Ret);
}

void UInventoryManagementLibrary::GiveItemEntryToInventoryOwnerHook(UObject* Context, FFrame& Stack, void* Ret)
{
	// Idk how to do the step with a itementry, allocate the size and then pass that pointer? idk.

	// TScriptInterface<UFortInventoryOwnerInterface>                             InventoryOwner;                                    // 0x0(0x10)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, NativeAccessSpecifierPublic)
	// __int64                        ItemEntry;

	return GiveItemEntryToInventoryOwnerOriginal(Context, Stack, Ret);
}

void UInventoryManagementLibrary::AddItemsHook(UObject* Context, FFrame& Stack, void* Ret)
{
	TScriptInterface<UFortInventoryOwnerInterface>                             InventoryOwner;                                    // 0x0(0x10)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, NativeAccessSpecifierPublic)
	TArray<FItemAndCount>                 Items;

	Stack.StepCompiledIn(&InventoryOwner);
	Stack.StepCompiledIn(&Items);

	auto ObjectPointer = InventoryOwner.ObjectPointer;

	auto PlayerController = Cast<AFortPlayerController>(ObjectPointer);

	LOG_INFO(LogDev, "Giving {} items to {}.", Items.Num(), __int64(PlayerController));

	if (!PlayerController)
		return AddItemsOriginal(Context, Stack, Ret);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return;

	for (int i = 0; i < Items.Num(); ++i)
	{
		WorldInventory->AddItem(Items.at(i).GetItem(), nullptr, Items.at(i).GetCount());
	}

	WorldInventory->Update();

	return AddItemsOriginal(Context, Stack, Ret);
}

void UInventoryManagementLibrary::AddItemHook(UObject* Context, FFrame& Stack, void* Ret)
{
	TScriptInterface<UFortInventoryOwnerInterface> InventoryOwner;                                    // 0x0(0x10)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, NativeAccessSpecifierPublic)
	UFortItemDefinition* ItemDefinition;
	int32                                        Count;                                             // 0x18(0x4)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	Stack.StepCompiledIn(&InventoryOwner);
	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&Count);

	LOG_INFO(LogDev, "[{}] ItemDefinition: {}", __FUNCTION__, __int64(ItemDefinition));

	if (!ItemDefinition)
		return AddItemOriginal(Context, Stack, Ret);

	auto ObjectPointer = InventoryOwner.ObjectPointer;

	auto PlayerController = Cast<AFortPlayerController>(ObjectPointer);

	if (!PlayerController)
		return AddItemOriginal(Context, Stack, Ret);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return;

	bool bShouldUpdate = false;
	WorldInventory->AddItem(ItemDefinition, &bShouldUpdate, bShouldUpdate, Count);

	if (bShouldUpdate)
		WorldInventory->Update();
	
	return AddItemOriginal(Context, Stack, Ret);
}
#include "FortMinigame.h"
#include "FortPlayerControllerAthena.h"

void AFortMinigame::ClearPlayerInventoryHook(UObject* Context, FFrame& Stack, void* Ret)
{
	auto Minigame = (AFortMinigame*)Context;

	if (!Minigame)
		return;

	AFortPlayerControllerAthena* PlayerController = nullptr;

	Stack.StepCompiledIn(&PlayerController);

	if (!PlayerController)
		return;

	auto& ItemInstances = PlayerController->GetWorldInventory()->GetItemList().GetItemInstances();

	for (int i = 0; i < ItemInstances.Num(); ++i)
	{
		auto ItemInstance = ItemInstances.at(i);

		PlayerController->GetWorldInventory()->RemoveItem(ItemInstance->GetItemEntry()->GetItemGuid(), nullptr, ItemInstance->GetItemEntry()->GetCount());
	}

	PlayerController->GetWorldInventory()->Update();

	return ClearPlayerInventoryOriginal(Context, Stack, Ret);
}
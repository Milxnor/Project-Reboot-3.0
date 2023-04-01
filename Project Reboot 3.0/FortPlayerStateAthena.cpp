#include "FortPlayerStateAthena.h"
#include "Stack.h"
#include "FortPlayerControllerAthena.h"

void AFortPlayerStateAthena::ServerSetInAircraftHook(UObject* Context, FFrame& Stack, void* Ret)
{
	auto PlayerState = (AFortPlayerStateAthena*)Context;
	auto PlayerController = Cast<AFortPlayerControllerAthena>(PlayerState->GetOwner());

	if (!PlayerController)
		return;

	// std::cout << "bNewInAircraft: " << bNewInAircraft << '\n';
	// std::cout << "PlayerController->IsInAircraft(): " << PlayerController->IsInAircraft() << '\n';

	struct aaa { bool wtf; };

	auto bNewInAircraft = ((aaa*)Stack.Locals)->wtf;// *(bool*)Stack.Locals;
	auto WorldInventory = PlayerController->GetWorldInventory();
	auto& InventoryList = WorldInventory->GetItemList();

	auto& ItemInstances = InventoryList.GetItemInstances();

	if ((/* (bNewInAircraft && !PlayerController->IsInAircraft()) || */ (Globals::bLateGame ? bNewInAircraft : true)) && ItemInstances.Num())
	{
		// std::cout << "InventoryList.ItemInstances.Num(): " << InventoryList.ItemInstances.Num() << '\n';

		std::vector<std::pair<FGuid, int>> GuidAndCountsToRemove;

		for (int i = 0; i < ItemInstances.Num(); i++)
		{
			auto ItemEntry = ItemInstances.at(i)->GetItemEntry();
			auto ItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry->GetItemDefinition());
			
			if (!ItemDefinition)
				continue;

			if (!ItemDefinition->CanBeDropped())
				continue;

			GuidAndCountsToRemove.push_back({ ItemEntry->GetItemGuid(), ItemEntry->GetCount() });
		}

		for (auto& Pair : GuidAndCountsToRemove)
		{
			WorldInventory->RemoveItem(Pair.first, nullptr, Pair.second, true);
		}

		WorldInventory->Update();

		static auto CurrentShieldOffset = PlayerState->GetOffset("CurrentShield");

		if (CurrentShieldOffset != -1)
			PlayerState->Get<float>(CurrentShieldOffset) = 0; // real
	}

	return ServerSetInAircraftOriginal(Context, Stack, Ret);
}
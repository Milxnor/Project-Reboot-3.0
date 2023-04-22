#include "FortPlayerStateAthena.h"
#include "Stack.h"
#include "FortPlayerControllerAthena.h"

void AFortPlayerStateAthena::ServerSetInAircraftHook(UObject* Context, FFrame& Stack, void* Ret)
{
	/* LOG_INFO(LogDev, "bLateGame: {}", Globals::bLateGame)

	if (Globals::bLateGame)
		return ServerSetInAircraftOriginal(Context, Stack, Ret); */

	auto PlayerState = (AFortPlayerStateAthena*)Context;
	auto PlayerController = Cast<AFortPlayerControllerAthena>(PlayerState->GetOwner());

	if (!PlayerController)
		return ServerSetInAircraftOriginal(Context, Stack, Ret);

	// std::cout << "PlayerController->IsInAircraft(): " << PlayerController->IsInAircraft() << '\n';

	struct aaa { bool wtf; };

	auto bNewInAircraft = ((aaa*)Stack.Locals)->wtf;// *(bool*)Stack.Locals;
	LOG_INFO(LogDev, "bNewInAircraft: {}", bNewInAircraft);
	auto WorldInventory = PlayerController->GetWorldInventory();
	auto& InventoryList = WorldInventory->GetItemList();

	auto& ItemInstances = InventoryList.GetItemInstances();

	bool bOverrideDontClearInventory = false;

	if (/* (bNewInAircraft && !PlayerController->IsInAircraft()) || */ /* (Globals::bLateGame ? bNewInAircraft : true)) && */
		!Globals::bLateGame.load() && ItemInstances.Num() && !bOverrideDontClearInventory)
	{
		static auto CurrentShieldOffset = PlayerState->GetOffset("CurrentShield");

		if (CurrentShieldOffset != -1)
			PlayerState->Get<float>(CurrentShieldOffset) = 0; // real
	}

	return ServerSetInAircraftOriginal(Context, Stack, Ret);
}
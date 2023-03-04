#pragma once

#include "FortPlayerController.h"
#include "FortPlayerStateAthena.h"

class AFortPlayerControllerAthena : public AFortPlayerController
{
public:
	static void ServerAcknowledgePossessionHook(APlayerController* Controller, APawn* Pawn)
	{
		static auto AcknowledgedPawnOffset = Controller->GetOffset("AcknowledgedPawn");
		Controller->Get<APawn*>(AcknowledgedPawnOffset) = Pawn;
	}

	AFortPlayerStateAthena* GetPlayerStateAthena()
	{
		return (AFortPlayerStateAthena*)GetPlayerState();
	}
};
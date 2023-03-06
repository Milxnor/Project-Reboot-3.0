#pragma once

#include "FortPlayerController.h"
#include "FortPlayerStateAthena.h"

class AFortPlayerControllerAthena : public AFortPlayerController
{
public:
	static inline void (*GetPlayerViewPointOriginal)(AFortPlayerControllerAthena* PlayerController, FVector& Location, FRotator& Rotation);

	AFortPlayerStateAthena* GetPlayerStateAthena()
	{
		return (AFortPlayerStateAthena*)GetPlayerState();
	}

	static void ServerAcknowledgePossessionHook(APlayerController* Controller, APawn* Pawn)
	{
		static auto AcknowledgedPawnOffset = Controller->GetOffset("AcknowledgedPawn");
		Controller->Get<APawn*>(AcknowledgedPawnOffset) = Pawn;
	}

	static void GetPlayerViewPointHook(AFortPlayerControllerAthena* PlayerController, FVector& Location, FRotator& Rotation);
};
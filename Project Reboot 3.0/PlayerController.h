#pragma once

#include "Class.h"
#include "Controller.h"
#include "CheatManager.h"

#include "Rotator.h"

class APlayerController : public AController
{
public:
	/* void Possess(APawn* Pawn)
	{
		static auto Possess = FindObject<UFunction>("/Script/Engine.Controller.Possess");
		this->ProcessEvent(Possess, &Pawn);
	}

	static inline void ServerAcknowledgePossessionHook(APlayerController* PlayerController, APawn* P)
	{
		static auto AcknowledgedPawnOffset = PlayerController->GetOffset("AcknowledgedPawn");
		PlayerController->Get<APawn*>(AcknowledgedPawnOffset) = P;
	} */

	class APlayerState*& GetPlayerState()
	{
		static auto PlayerStateOffset = this->GetOffset("PlayerState");
		return this->Get<class APlayerState*>(PlayerStateOffset);
	}

	class APawn*& GetPawn()
	{
		static auto PawnOffset = this->GetOffset("Pawn");
		return this->Get<class APawn*>(PawnOffset);
	}

	UCheatManager*& GetCheatManager()
	{
		static auto CheatManagerOffset = this->GetOffset("CheatManager");
		return this->Get<UCheatManager*>(CheatManagerOffset);
	}

	UCheatManager*& SpawnCheatManager(UClass* CheatManagerClass);
	FRotator GetControlRotation();
	void Possess(class APawn* Pawn);
	void ServerRestartPlayer();

	static UClass* StaticClass();
};
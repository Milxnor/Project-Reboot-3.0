#pragma once

#include "Actor.h"

class AController : public AActor
{
public:
	AActor* GetViewTarget();
	void Possess(class APawn* Pawn);

	class APawn*& GetPawn()
	{
		static auto PawnOffset = this->GetOffset("Pawn");
		return this->Get<class APawn*>(PawnOffset);
	}

	class APlayerState*& GetPlayerState()
	{
		static auto PlayerStateOffset = this->GetOffset("PlayerState");
		return this->Get<class APlayerState*>(PlayerStateOffset);
	}
};
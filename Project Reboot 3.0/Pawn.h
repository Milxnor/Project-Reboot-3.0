#pragma once

#include "Actor.h"
#include "TSubClassOf.h"

class APawn : public AActor
{
public:
	UObject* GetPlayerState()
	{
		static auto PlayerStateOffset = GetOffset("PlayerState");
		return Get(PlayerStateOffset);
	}

	class APlayerController* GetController()
	{
		static auto ControllerOffset = GetOffset("Controller");
		return Get<class APlayerController*>(ControllerOffset);
	}

	TSubclassOf<class AController> GetAIControllerClass()
	{
		static auto AIControllerClassOffset = GetOffset("AIControllerClass");
		return Get<TSubclassOf<class AController>>(AIControllerClassOffset);
	}
};
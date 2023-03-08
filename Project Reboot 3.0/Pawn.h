#pragma once

#include "Actor.h"

class APawn : public AActor
{
public:
	UObject* GetPlayerState()
	{
		static auto PlayerStateOffset = GetOffset("PlayerState");
		return Get(PlayerStateOffset);
	}
};
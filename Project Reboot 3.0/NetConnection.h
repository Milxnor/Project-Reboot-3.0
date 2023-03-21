#pragma once

#include "Player.h"

class UNetConnection : public UPlayer
{
public:
	AActor*& GetOwningActor()
	{
		static auto OwningActorOffset = GetOffset("OwningActor");
		return Get<AActor*>(OwningActorOffset);
	}

	AActor*& GetViewTarget()
	{
		static auto ViewTargetOffset = GetOffset("ViewTarget");
		return Get<AActor*>(ViewTargetOffset);
	}
};
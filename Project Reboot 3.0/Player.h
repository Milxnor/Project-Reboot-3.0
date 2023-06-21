#pragma once

#include "Object.h"
#include "PlayerController.h"

class UPlayer : public UObject
{
public:
	APlayerController*& GetPlayerController() const
	{
		static auto PlayerControllerOffset = GetOffset("PlayerController");
		return Get<APlayerController*>(PlayerControllerOffset);
	}

	int& GetCurrentNetSpeed()
	{
		static auto CurrentNetSpeedOffset = GetOffset("CurrentNetSpeed");
		return Get<int>(CurrentNetSpeedOffset);
	}
};
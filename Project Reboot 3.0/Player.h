#pragma once

#include "Object.h"
#include "PlayerController.h"

class UPlayer : public UObject
{
public:
	APlayerController*& GetPlayerController()
	{
		static auto PlayerControllerOffset = GetOffset("PlayerController");
		return Get<APlayerController*>(PlayerControllerOffset);
	}
};
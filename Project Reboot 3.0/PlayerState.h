#pragma once

#include "Actor.h"

#include "UnrealString.h"

class APlayerState : public AActor
{
public:
	FString GetPlayerName();
	int& GetPlayerID(); // for future me to deal with (this is a short on some versions).
};
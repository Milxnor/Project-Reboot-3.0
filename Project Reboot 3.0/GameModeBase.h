#pragma once

#include "Actor.h"

#include "Controller.h"
#include "Pawn.h"

class AGameModeBase : public AActor // AInfo
{
public:
	static APawn* SpawnDefaultPawnForHook(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot);
};
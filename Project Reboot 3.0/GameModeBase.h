#pragma once

#include "Actor.h"

#include "Controller.h"
#include "Pawn.h"
#include "UnrealString.h"

class AGameModeBase : public AActor // AInfo
{
public:
	UClass* GetDefaultPawnClassForController(AController* InController);
	void ChangeName(AController* Controller, const FString& NewName, bool bNameChange);
	AActor* K2_FindPlayerStart(AController* Player, FString IncomingName);
	void RestartPlayerAtTransform(AController* NewPlayer, FTransform SpawnTransform);
	void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot);
	void RestartPlayer(AController* NewPlayer);

	static APawn* SpawnDefaultPawnForHook(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot);
};
#pragma once

#include "Actor.h"

#include "Controller.h"
#include "Pawn.h"
#include "UnrealString.h"
#include "Stack.h"

class AGameModeBase : public AActor // AInfo
{
public:
	static inline bool (*PlayerCanRestartOriginal)(UObject* Context, FFrame& Stack, bool* Ret);

	UClass* GetDefaultPawnClassForController(AController* InController);
	void ChangeName(AController* Controller, const FString& NewName, bool bNameChange);
	AActor* K2_FindPlayerStart(AController* Player, FString IncomingName);
	void RestartPlayerAtTransform(AController* NewPlayer, FTransform SpawnTransform);
	void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot);
	void RestartPlayer(AController* NewPlayer);

	static bool PlayerCanRestartHook(UObject* Context, FFrame& Stack, bool* Ret);
	static APawn* SpawnDefaultPawnForHook(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot);
};
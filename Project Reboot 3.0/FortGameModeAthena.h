#pragma once

#include "FortGameModePvPBase.h"
// #include "FortPlayerControllerAthena.h"
#include "FortGameStateAthena.h"

class AFortGameModeAthena : public AFortGameModePvPBase
{
public:
	static inline bool (*Athena_ReadyToStartMatchOriginal)(AFortGameModeAthena* GameMode);
	static inline void (*Athena_HandleStartingNewPlayerOriginal)(AFortGameModeAthena* GameMode, AActor* NewPlayer);

	static bool Athena_ReadyToStartMatchHook(AFortGameModeAthena* GameMode);
	static int Athena_PickTeamHook(AFortGameModeAthena* GameMode, uint8 preferredTeam, AActor* Controller);
	static void Athena_HandleStartingNewPlayerHook(AFortGameModeAthena* GameMode, AActor* NewPlayerActor);

	AFortGameStateAthena* GetGameStateAthena()
	{
		return (AFortGameStateAthena*)GetGameState();
	}
};
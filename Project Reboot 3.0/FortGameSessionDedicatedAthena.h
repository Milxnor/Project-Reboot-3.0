#pragma once

#include "reboot.h"

class AFortGameSessionDedicatedAthena : public AActor
{
public:
	static uint8 GetSquadIdForCurrentPlayerHook(AFortGameSessionDedicatedAthena* GameSessionDedicated, void* UniqueId);
};
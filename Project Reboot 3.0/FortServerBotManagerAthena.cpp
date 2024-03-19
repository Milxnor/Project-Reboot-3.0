#include "FortServerBotManagerAthena.h"
#include "bots.h"

AFortPlayerPawnAthena* UFortServerBotManagerAthena::SpawnBotHook(UFortServerBotManagerAthena* BotManager, FVector& InSpawnLocation, FRotator& InSpawnRotation,
	UFortAthenaAIBotCustomizationData* InBotData, FFortAthenaAIBotRunTimeCustomizationData* InRuntimeBotData)
{
	LOG_INFO(LogBots, "SpawnBotHook!");

	return nullptr;
}
#include "FortServerBotManagerAthena.h"
#include "bots.h"
#include "ai.h"
#include "finder.h"

AFortPlayerPawnAthena* UFortServerBotManagerAthena::SpawnBotHook(UFortServerBotManagerAthena* BotManager, FVector& InSpawnLocation, FRotator& InSpawnRotation,
	UFortAthenaAIBotCustomizationData* InBotData, FFortAthenaAIBotRunTimeCustomizationData& InRuntimeBotData)
{
	LOG_INFO(LogBots, "SpawnBotHook!");

	if (__int64(_ReturnAddress()) == SpawnBotRet()) {
		return SpawnBotOriginal(BotManager, InSpawnLocation, InSpawnRotation, InBotData, InRuntimeBotData);
	}

	AActor* SpawnLocator = GetWorld()->SpawnActor<APawn>(APawn::StaticClass(), InSpawnLocation, InSpawnRotation.Quaternion());
	auto PawnClass = InBotData->Get<UClass*>(InBotData->GetOffset("PawnClass"));
	auto Pawn = BotMutator->SpawnBot(PawnClass , SpawnLocator, InSpawnLocation, InSpawnRotation, true);

	Pawn->TeleportTo(InSpawnLocation, InSpawnRotation);//idfk????

	return Pawn;
}
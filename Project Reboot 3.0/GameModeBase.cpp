#include "GameModeBase.h"

#include "reboot.h"
#include "FortPlayerController.h"

APawn* AGameModeBase::SpawnDefaultPawnForHook(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot)
{
	LOG_INFO(LogDev, "SpawnDefaultPawnFor: 0x{:x}!", __int64(_ReturnAddress()) - __int64(GetModuleHandleW(0)));

	static auto PawnClass = FindObject<UClass>("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
	GameMode->Get<UClass*>("DefaultPawnClass") = PawnClass;

	static auto fn = FindObject<UFunction>(L"/Script/Engine.GameModeBase.SpawnDefaultPawnAtTransform");

	struct { AController* NewPlayer; FTransform SpawnTransform; APawn* ReturnValue; } 
	AGameModeBase_SpawnDefaultPawnAtTransform_Params{NewPlayer, StartSpot->GetTransform()};

	GameMode->ProcessEvent(fn, &AGameModeBase_SpawnDefaultPawnAtTransform_Params);

	return AGameModeBase_SpawnDefaultPawnAtTransform_Params.ReturnValue;
}
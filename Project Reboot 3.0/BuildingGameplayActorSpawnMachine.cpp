#include "BuildingGameplayActorSpawnMachine.h"

#include "FortPlayerControllerAthena.h"
#include "GameplayStatics.h"
#include "AthenaResurrectionComponent.h"
#include "FortGameStateAthena.h"
#include "FortGameModeAthena.h"

void ABuildingGameplayActorSpawnMachine::FinishResurrection(int SquadId)
{
	static void (*FinishResurrectionOriginal)(ABuildingGameplayActorSpawnMachine* SpawnMachine, int SquadId) = decltype(FinishResurrectionOriginal)(Addresses::FinishResurrection);

	if (FinishResurrectionOriginal)
	{
		FinishResurrectionOriginal(this, SquadId);
	}
	else
	{

	}
}

void ABuildingGameplayActorSpawnMachine::RebootingDelegateHook(ABuildingGameplayActorSpawnMachine* SpawnMachine)
{
	auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

	LOG_INFO(LogDev, "RebootingDelegateHook!");

	if (!SpawnMachine->GetResurrectLocation())
		return;

	LOG_INFO(LogDev, "PlayerIdsForResurrection.Num(): {}", SpawnMachine->GetPlayerIdsForResurrection().Num());

	if (SpawnMachine->GetPlayerIdsForResurrection().Num() <= 0)
		return;

	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	AFortPlayerControllerAthena* PlayerController = nullptr;

	if (auto TeamArrayContainer = GameState->GetTeamsArrayContainer())
	{
		auto& SquadArray = TeamArrayContainer->SquadsArray.at(SpawnMachine->GetSquadId());

		for (int i = 0; i < SquadArray.Num(); i++)
		{
			auto StrongPlayerState = SquadArray.at(i).Get();

			if (!StrongPlayerState)
				continue;

			PlayerController = Cast<AFortPlayerControllerAthena>(StrongPlayerState->GetOwner());

			if (!PlayerController)
				continue;

			if (PlayerController->InternalIndex == SpawnMachine->GetInstigatorPC().ObjectIndex)
				continue;

			break;
		}
	}

	LOG_INFO(LogDev, "PlayerController: {}", __int64(PlayerController));

	if (!PlayerController)
		return;

	auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState());

	if (!PlayerState)
		return;

	auto ResurrectionComponent = PlayerController->GetResurrectionComponent();

	if (!ResurrectionComponent)
		return;

	static auto FortPlayerStartClass = FindObject<UClass>(L"/Script/FortniteGame.FortPlayerStart");

	if (true) // i dont think we actually need this
	{
		ResurrectionComponent->GetResurrectionLocation().ObjectIndex = SpawnMachine->GetResurrectLocation()->InternalIndex;
		ResurrectionComponent->GetResurrectionLocation().ObjectSerialNumber = GetItemByIndex(SpawnMachine->GetResurrectLocation()->InternalIndex)->SerialNumber;
	}

	auto StrongResurrectionLocation = ResurrectionComponent->GetResurrectionLocation().Get();

	LOG_INFO(LogDev, "StrongResurrectionLocation: {} IsRespawnDataAvailable: {}", __int64(StrongResurrectionLocation), PlayerState->GetRespawnData()->IsRespawnDataAvailable());

	if (!StrongResurrectionLocation)
		return;

	// GameMode->RestartPlayerAtPlayerStart(PlayerController, StrongResurrectionLocation);

	PlayerState->GetRespawnData()->IsRespawnDataAvailable() = false;
	PlayerController->SetPlayerIsWaiting(true);
	PlayerController->ServerRestartPlayer();

	/* static auto PawnClass = FindObject<UClass>("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
	auto NewPawn = GetWorld()->SpawnActor<AFortPlayerPawnAthena>(PawnClass, StrongResurrectionLocation->GetTransform());
	PlayerController->Possess(NewPawn); */

	AFortPlayerPawn* NewPawn = Cast<AFortPlayerPawn>(PlayerController->GetMyFortPawn());

	LOG_INFO(LogDev, "NewPawn: {}", __int64(NewPawn));

	if (!NewPawn) // Failed to restart player
		return;

	bool bEnterSkydiving = false; // TODO get from like curve table iirc idk or the variable

	PlayerController->ClientClearDeathNotification();
	// PlayerController->RespawnPlayerAfterDeath(bEnterSkydiving);

	NewPawn->SetHealth(100);
	NewPawn->SetMaxHealth(100);

	static auto RebootCounterOffset = PlayerState->GetOffset("RebootCounter");
	PlayerState->Get<int>(RebootCounterOffset)++;

	static auto OnRep_RebootCounterFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerStateAthena.OnRep_RebootCounter");
	PlayerState->ProcessEvent(OnRep_RebootCounterFn);

	static void (*AddToAlivePlayersOriginal)(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Player) = decltype(AddToAlivePlayersOriginal)(Addresses::AddToAlivePlayers);

	if (AddToAlivePlayersOriginal)
	{
		AddToAlivePlayersOriginal(GameMode, PlayerController);
	}

	auto OnPlayerPawnResurrectedFn = SpawnMachine->FindFunction("OnPlayerPawnResurrected");
	SpawnMachine->ProcessEvent(OnPlayerPawnResurrectedFn, &NewPawn);

	bool IsFinalPlayerToBeRebooted = true;

	if (IsFinalPlayerToBeRebooted)
	{
		SpawnMachine->FinishResurrection(PlayerState->GetSquadId());
	}
}
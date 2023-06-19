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
	{
		LOG_WARN(LogRebooting, "Reboot van did not have a resurrection location!");
		return;
	}

	LOG_INFO(LogDev, "PlayerIdsForResurrection.Num(): {}", SpawnMachine->GetPlayerIdsForResurrection().Num());

	if (SpawnMachine->GetPlayerIdsForResurrection().Num() <= 0)
		return;

	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	AFortPlayerControllerAthena* PlayerController = nullptr;

	if (auto TeamArrayContainer = GameState->GetTeamsArrayContainer())
	{
		auto& SquadArray = TeamArrayContainer->SquadsArray.at(SpawnMachine->GetSquadId());

		for (int i = 0; i < SquadArray.Num(); ++i)
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

	PlayerState->GetRespawnData()->IsRespawnDataAvailable() = false;
	PlayerController->SetPlayerIsWaiting(true);
	// PlayerController->ServerRestartPlayer();

	bool bEnterSkydiving = false; // TODO get from like curve table iirc idk or the variable
	PlayerController->RespawnPlayerAfterDeath(bEnterSkydiving);

	AFortPlayerPawn* NewPawn = Cast<AFortPlayerPawn>(PlayerController->GetMyFortPawn());

	LOG_INFO(LogDev, "NewPawn: {}", __int64(NewPawn));

	if (!NewPawn) // Failed to restart player
	{
		LOG_INFO(LogRebooting, "Failed to restart the player!");
		return;
	}

	PlayerController->ClientClearDeathNotification();

	NewPawn->SetHealth(100);
	NewPawn->SetMaxHealth(100);

	static auto RebootCounterOffset = PlayerState->GetOffset("RebootCounter");
	PlayerState->Get<int>(RebootCounterOffset)++;

	static auto OnRep_RebootCounterFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerStateAthena.OnRep_RebootCounter");
	PlayerState->ProcessEvent(OnRep_RebootCounterFn);

	auto OnPlayerPawnResurrectedFn = SpawnMachine->FindFunction("OnPlayerPawnResurrected");
	SpawnMachine->ProcessEvent(OnPlayerPawnResurrectedFn, &NewPawn);

	static void (*AddToAlivePlayersOriginal)(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Player) = decltype(AddToAlivePlayersOriginal)(Addresses::AddToAlivePlayers);

	if (AddToAlivePlayersOriginal)
	{
		AddToAlivePlayersOriginal(GameMode, PlayerController);
	}

	bool IsFinalPlayerToBeRebooted = true;

	if (IsFinalPlayerToBeRebooted)
	{
		SpawnMachine->FinishResurrection(PlayerState->GetSquadId());
	}
}
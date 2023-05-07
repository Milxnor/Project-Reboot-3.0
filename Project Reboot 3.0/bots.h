#pragma once

#include "FortGameModeAthena.h"
#include "OnlineReplStructs.h"
#include "BuildingContainer.h"

class PlayerBot
{
public:
	AFortPlayerControllerAthena* PlayerController = nullptr;
	float NextJumpTime = 1.0f;

	void Initialize(FTransform SpawnTransform)
	{
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		static auto PawnClass = FindObject<UClass>("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");

		FActorSpawnParameters PawnSpawnParameters{};
		PawnSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		PlayerController = GetWorld()->SpawnActor<AFortPlayerControllerAthena>(AFortPlayerControllerAthena::StaticClass());
		AFortPlayerPawnAthena* Pawn = GetWorld()->SpawnActor<AFortPlayerPawnAthena>(PawnClass, SpawnTransform, PawnSpawnParameters);
		AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState());

		if (!Pawn || !PlayerState)
			return;

		static int CurrentBotNum = 1;

		auto BotNumWStr = std::to_wstring(CurrentBotNum++);

		FString NewName = (L"RebootBot" + BotNumWStr).c_str();

		PlayerController->ServerChangeName(NewName);
		PlayerState->OnRep_PlayerName();

		PlayerState->GetTeamIndex() = GameMode->Athena_PickTeamHook(GameMode, 0, PlayerController);

		static auto SquadIdOffset = PlayerState->GetOffset("SquadId", false);

		if (SquadIdOffset != -1)
			PlayerState->GetSquadId() = PlayerState->GetTeamIndex() - 2;

		PlayerState->SetIsBot(true);

		/*

		static auto FortRegisteredPlayerInfoClass = FindObject<UClass>("/Script/FortniteGame.FortRegisteredPlayerInfo");
		static auto MyPlayerInfoOffset = PlayerController->GetOffset("MyPlayerInfo");
		PlayerController->Get(MyPlayerInfoOffset) = UGameplayStatics::SpawnObject(FortRegisteredPlayerInfoClass, PlayerController);

		if (!PlayerController->Get(MyPlayerInfoOffset))
		{
			LOG_ERROR(LogBots, "Failed to spawn PlayerInfo!");

			Pawn->K2_DestroyActor();
			PlayerController->K2_DestroyActor();
			return nullptr;
		}

		auto& PlayerInfo = PlayerController->Get(MyPlayerInfoOffset);

		static auto UniqueIdOffset = PlayerState->GetOffset("UniqueId");
		static auto PlayerInfo_PlayerNameOffset = PlayerInfo->GetOffset("PlayerName");
		static auto PlayerIDOffset = PlayerInfo->GetOffset("PlayerID");
		PlayerInfo->GetPtr<FUniqueNetIdRepl>(PlayerIDOffset)->CopyFromAnotherUniqueId(PlayerState->GetPtr<FUniqueNetIdRepl>(UniqueIdOffset));
		PlayerInfo->Get<FString>(PlayerInfo_PlayerNameOffset) = PlayerState->GetPlayerName();

		*/

		PlayerController->Possess(Pawn);

		Pawn->SetHealth(100);
		Pawn->SetMaxHealth(100);

		FActorSpawnParameters WorldInventorySpawnParameters{};
		WorldInventorySpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		WorldInventorySpawnParameters.Owner = PlayerController;

		FTransform WorldInventorySpawnTransform{};

		static auto FortInventoryClass = FindObject<UClass>("/Script/FortniteGame.FortInventory"); // AFortInventory::StaticClass()
		PlayerController->GetWorldInventory() = GetWorld()->SpawnActor<AFortInventory>(FortInventoryClass, WorldInventorySpawnTransform, WorldInventorySpawnParameters);

		if (!PlayerController->GetWorldInventory())
		{
			LOG_ERROR(LogBots, "Failed to spawn WorldInventory!");

			Pawn->K2_DestroyActor();
			PlayerController->K2_DestroyActor();
			return;
		}

		PlayerController->GetWorldInventory()->GetInventoryType() = EFortInventoryType::World;

		static auto bHasInitializedWorldInventoryOffset = PlayerController->GetOffset("bHasInitializedWorldInventory");
		PlayerController->Get<bool>(bHasInitializedWorldInventoryOffset) = true;

		auto& StartingItems = GameMode->GetStartingItems();

		auto PickaxeInstance = PlayerController->AddPickaxeToInventory();

		for (int i = 0; i < StartingItems.Num(); i++)
		{
			auto& StartingItem = StartingItems.at(i);

			PlayerController->GetWorldInventory()->AddItem(StartingItem.GetItem(), nullptr, StartingItem.GetCount());
		}

		if (PickaxeInstance)
		{
			PlayerController->ServerExecuteInventoryItemHook(PlayerController, PickaxeInstance->GetItemEntry()->GetItemGuid());
		}

		PlayerController->GetWorldInventory()->Update();

		/* static auto HeroType = FindObject(L"/Game/Athena/Heroes/HID_115_Athena_Commando_M_CarbideBlue.HID_115_Athena_Commando_M_CarbideBlue");

		static auto HeroTypeOffset = PlayerState->GetOffset("HeroType");

		if (HeroTypeOffset != -1)
			PlayerState->Get(HeroTypeOffset) = HeroType; */

		auto PlayerAbilitySet = GetPlayerAbilitySet();
		auto AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

		if (PlayerAbilitySet)
		{
			PlayerAbilitySet->GiveToAbilitySystem(AbilitySystemComponent);
		}

		PlayerController->GetCosmeticLoadout()->GetCharacter() = FindObject("/Game/Athena/Items/Cosmetics/Characters/CID_263_Athena_Commando_F_MadCommander.CID_263_Athena_Commando_F_MadCommander");
		Pawn->GetCosmeticLoadout()->GetCharacter() = PlayerController->GetCosmeticLoadout()->GetCharacter();

		PlayerController->ApplyCosmeticLoadout();

		GameState->GetPlayersLeft()++;
		GameState->OnRep_PlayersLeft();

		GameMode->GetAlivePlayers().Add(PlayerController);
	}
};

static std::vector<PlayerBot> AllPlayerBotsToTick;

namespace Bots
{
	static AFortPlayerControllerAthena* SpawnBot(FTransform SpawnTransform)
	{
		auto playerBot = PlayerBot();
		playerBot.Initialize(SpawnTransform);
		AllPlayerBotsToTick.push_back(playerBot);
		return playerBot.PlayerController;
	}

	static void SpawnBotsAtPlayerStarts(int AmountOfBots)
	{
		return;

		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		for (int i = 0; i < AmountOfBots; i++)
		{
			FTransform SpawnTransform{};
			SpawnTransform.Translation = FVector(1, 1, 10000);
			SpawnTransform.Rotation = FQuat();
			SpawnTransform.Scale3D = FVector(1, 1, 1);

			auto NewBot = SpawnBot(SpawnTransform);
			auto PlayerStart = GameMode->K2_FindPlayerStart(NewBot, NewBot->GetPlayerState()->GetPlayerName()); // i dont think this works

			if (!PlayerStart)
			{
				LOG_ERROR(LogBots, "Failed to find PlayerStart for bot!");
				NewBot->GetPawn()->K2_DestroyActor();
				NewBot->K2_DestroyActor();
				continue;
			}

			NewBot->TeleportTo(PlayerStart->GetActorLocation(), FRotator());
			NewBot->SetCanBeDamaged(Fortnite_Version < 7); // idk lol for spawn island
		}
	}

	static void Tick()
	{
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		auto AllBuildingContainers = UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingContainer::StaticClass());

		// for (int i = 0; i < GameMode->GetAlivePlayers().Num(); i++)
		for (auto& PlayerBot : AllPlayerBotsToTick)
		{
			auto CurrentPlayer = PlayerBot.PlayerController;

			if (CurrentPlayer->IsActorBeingDestroyed())
				continue;

			auto CurrentPawn = CurrentPlayer->GetMyFortPawn();

			auto CurrentPlayerState = Cast<AFortPlayerStateAthena>(CurrentPlayer->GetPlayerState());

			if (!CurrentPlayerState || !CurrentPlayerState->IsBot())
				continue;

			if (GameState->GetGamePhase() == EAthenaGamePhase::Warmup)
			{
				/* if (!CurrentPlayer->IsPlayingEmote())
				{
					static auto AthenaDanceItemDefinitionClass = FindObject<UClass>("/Script/FortniteGame.AthenaDanceItemDefinition");
					auto RandomDanceID = GetRandomObjectOfClass(AthenaDanceItemDefinitionClass);

					CurrentPlayer->ServerPlayEmoteItemHook(CurrentPlayer, RandomDanceID);
				} */
			}	

			if (CurrentPlayerState->IsInAircraft() && !CurrentPlayerState->HasThankedBusDriver())
			{
				static auto ServerThankBusDriverFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerControllerAthena.ServerThankBusDriver");
				CurrentPlayer->ProcessEvent(ServerThankBusDriverFn);
			}

			if (CurrentPawn)
			{
				if (PlayerBot.NextJumpTime <= UGameplayStatics::GetTimeSeconds(GetWorld()))
				{
					static auto JumpFn = FindObject<UFunction>("/Script/Engine.Character.Jump");

					CurrentPawn->ProcessEvent(JumpFn);
					PlayerBot.NextJumpTime = UGameplayStatics::GetTimeSeconds(GetWorld()) + (rand() % 4 + 3);
				}
			}

			/* bool bShouldJumpFromBus = CurrentPlayerState->IsInAircraft(); // TODO (Milxnor) add a random percent thing

			if (bShouldJumpFromBus)
			{
				CurrentPlayer->ServerAttemptAircraftJumpHook(CurrentPlayer, FRotator());
			} */
		}

		AllBuildingContainers.Free();
	}
}
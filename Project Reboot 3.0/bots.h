#pragma once

#include "FortGameModeAthena.h"
#include "OnlineReplStructs.h"
#include "BuildingContainer.h"

class BotPOI
{
	FVector CenterLocation;
	FVector Range; // this just has to be FVector2D
};

class BotPOIEncounter
{
public:
	int NumChestsSearched;
	int NumAmmoBoxesSearched;
	int NumPlayersEncountered;
};

class PlayerBot
{
public:
	AController* Controller = nullptr;
	BotPOIEncounter currentBotEncounter;
	int TotalPlayersEncountered;
	std::vector<BotPOI> POIsTraveled;
	float NextJumpTime = 1.0f;

	void OnPlayerEncountered()
	{
		currentBotEncounter.NumPlayersEncountered++;
		TotalPlayersEncountered++;
	}

	void MoveToNewPOI()
	{

	}

	void Initialize(const FTransform& SpawnTransform)
	{
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		static UClass* PawnClass = nullptr;
		static UClass* ControllerClass = nullptr;

		bool bUsePhoebeClasses = false;

		if (!PawnClass)
		{
			if (!bUsePhoebeClasses)
				PawnClass = FindObject<UClass>(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
			else
				PawnClass = FindObject<UClass>(L"/Game/Athena/AI/Phoebe/BP_PlayerPawn_Athena_Phoebe.BP_PlayerPawn_Athena_Phoebe_C");
		}

		if (!ControllerClass)
		{
			if (!bUsePhoebeClasses)
				ControllerClass = AFortPlayerControllerAthena::StaticClass();
			else
				ControllerClass = FindObject<UClass>(L"/Game/Athena/AI/Phoebe/BP_PhoebePlayerController.BP_PhoebePlayerController_C");
		}

		if (!ControllerClass || !PawnClass)
		{
			LOG_ERROR(LogBots, "Failed to find a class for the bots!");
			return;
		}

		static auto FortAthenaAIBotControllerClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotController");

		Controller = GetWorld()->SpawnActor<AController>(ControllerClass);
		AFortPlayerPawnAthena* Pawn = GetWorld()->SpawnActor<AFortPlayerPawnAthena>(PawnClass, SpawnTransform, CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));
		AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Controller->GetPlayerState());

		if (!Pawn || !PlayerState)
			return;

		bool bUseOverrideName = false;

		FString NewName;

		if (bUseOverrideName)
		{
			NewName = L"Override";
		}
		else
		{
			static int CurrentBotNum = 1;
			auto BotNumWStr = std::to_wstring(CurrentBotNum++);
			NewName = (L"RebootBot" + BotNumWStr).c_str();
		}

		if (auto PlayerController = Cast<APlayerController>(Controller))
			PlayerController->ServerChangeName(NewName);

		PlayerState->OnRep_PlayerName();

		PlayerState->GetTeamIndex() = GameMode->Athena_PickTeamHook(GameMode, 0, Controller);

		static auto SquadIdOffset = PlayerState->GetOffset("SquadId", false);

		if (SquadIdOffset != -1)
			PlayerState->GetSquadId() = PlayerState->GetTeamIndex() - NumToSubtractFromSquadId;

		GameState->AddPlayerStateToGameMemberInfo(PlayerState);

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

		Controller->Possess(Pawn);

		Pawn->SetHealth(100);
		Pawn->SetMaxHealth(100);

		AFortInventory** Inventory = nullptr;

		if (auto FortPlayerController = Cast<AFortPlayerController>(Controller))
		{
			Inventory = &FortPlayerController->GetWorldInventory();
		}
		else
		{
			if (Controller->IsA(FortAthenaAIBotControllerClass))
			{
				static auto InventoryOffset = Controller->GetOffset("Inventory");
				Inventory = Controller->GetPtr<AFortInventory*>(InventoryOffset);
			}
		}

		if (!Inventory)
		{
			LOG_ERROR(LogBots, "No inventory pointer!");

			Pawn->K2_DestroyActor();
			Controller->K2_DestroyActor();
			return;
		}

		FTransform InventorySpawnTransform{};

		static auto FortInventoryClass = FindObject<UClass>(L"/Script/FortniteGame.FortInventory"); // AFortInventory::StaticClass()
		*Inventory = GetWorld()->SpawnActor<AFortInventory>(FortInventoryClass, InventorySpawnTransform, CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AlwaysSpawn, false, Controller));

		if (!*Inventory)
		{
			LOG_ERROR(LogBots, "Failed to spawn Inventory!");

			Pawn->K2_DestroyActor();
			Controller->K2_DestroyActor();
			return;
		}

		(*Inventory)->GetInventoryType() = EFortInventoryType::World;

		if (auto FortPlayerController = Cast<AFortPlayerController>(Controller))
		{
			static auto bHasInitializedWorldInventoryOffset = FortPlayerController->GetOffset("bHasInitializedWorldInventory");
			FortPlayerController->Get<bool>(bHasInitializedWorldInventoryOffset) = true;
		}

		// if (false)
		{
			if (Inventory)
			{
				auto& StartingItems = GameMode->GetStartingItems();

				for (int i = 0; i < StartingItems.Num(); ++i)
				{
					auto& StartingItem = StartingItems.at(i);

					(*Inventory)->AddItem(StartingItem.GetItem(), nullptr, StartingItem.GetCount());
				}

				if (auto FortPlayerController = Cast<AFortPlayerController>(Controller))
				{
					UFortItem* PickaxeInstance = FortPlayerController->AddPickaxeToInventory();

					if (PickaxeInstance)
					{
						FortPlayerController->ServerExecuteInventoryItemHook(FortPlayerController, PickaxeInstance->GetItemEntry()->GetItemGuid());
					}
				}

				(*Inventory)->Update();
			}
		}

		auto PlayerAbilitySet = GetPlayerAbilitySet();
		auto AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

		if (PlayerAbilitySet)
		{
			PlayerAbilitySet->GiveToAbilitySystem(AbilitySystemComponent);
		}

		// PlayerController->GetCosmeticLoadout()->GetCharacter() = FindObject("/Game/Athena/Items/Cosmetics/Characters/CID_263_Athena_Commando_F_MadCommander.CID_263_Athena_Commando_F_MadCommander");
		// Pawn->GetCosmeticLoadout()->GetCharacter() = PlayerController->GetCosmeticLoadout()->GetCharacter();

		// PlayerController->ApplyCosmeticLoadout();

		/*

		auto AllHeroTypes = GetAllObjectsOfClass(FindObject<UClass>(L"/Script/FortniteGame.FortHeroType"));
		std::vector<UFortItemDefinition*> AthenaHeroTypes;

		UFortItemDefinition* HeroType = FindObject<UFortItemDefinition>(L"/Game/Athena/Heroes/HID_030_Athena_Commando_M_Halloween.HID_030_Athena_Commando_M_Halloween");

		for (int i = 0; i < AllHeroTypes.size(); ++i)
		{
			auto CurrentHeroType = (UFortItemDefinition*)AllHeroTypes.at(i);

			if (CurrentHeroType->GetPathName().starts_with("/Game/Athena/Heroes/"))
				AthenaHeroTypes.push_back(CurrentHeroType);
		}

		if (AthenaHeroTypes.size())
		{
			HeroType = AthenaHeroTypes.at(std::rand() % AthenaHeroTypes.size());
		}

		static auto HeroTypeOffset = PlayerState->GetOffset("HeroType");

		if (HeroTypeOffset != -1)
			PlayerState->Get(HeroTypeOffset) = HeroType;

		static auto OwningGameInstanceOffset = GetWorld()->GetOffset("OwningGameInstance");
		auto OwningGameInstance = GetWorld()->Get(OwningGameInstanceOffset);

		static auto RegisteredPlayersOffset = OwningGameInstance->GetOffset("RegisteredPlayers");
		auto& RegisteredPlayers = OwningGameInstance->Get<TArray<UObject*>>(RegisteredPlayersOffset);

		static auto FortRegisteredPlayerInfoClass = FindObject<UClass>("/Script/FortniteGame.FortRegisteredPlayerInfo");

		auto NewPlayerInfo = UGameplayStatics::SpawnObject(FortRegisteredPlayerInfoClass, Controller);
		static auto PlayerIDOffset = NewPlayerInfo->GetOffset("PlayerID");

		static auto UniqueIdOffset = PlayerState->GetOffset("UniqueId");
		auto PlayerStateUniqueId = PlayerState->GetPtr<FUniqueNetIdRepl>(UniqueIdOffset);

		NewPlayerInfo->GetPtr<FUniqueNetIdRepl>(PlayerIDOffset)->CopyFromAnotherUniqueId(PlayerStateUniqueId);

		static auto MyPlayerInfoOffset = Controller->GetOffset("MyPlayerInfo");
		Controller->Get(MyPlayerInfoOffset) = NewPlayerInfo;

		RegisteredPlayers.Add(NewPlayerInfo);

		ApplyHID(Pawn, HeroType, true);

		*/

		GameState->GetPlayersLeft()++;
		GameState->OnRep_PlayersLeft();

		if (auto FortPlayerControllerAthena = Cast<AFortPlayerControllerAthena>(Controller))
			GameMode->GetAlivePlayers().Add(FortPlayerControllerAthena);
	}
};

static inline std::vector<PlayerBot> AllPlayerBotsToTick;

namespace Bots
{
	static AController* SpawnBot(FTransform SpawnTransform)
	{
		auto playerBot = PlayerBot();
		playerBot.Initialize(SpawnTransform);
		AllPlayerBotsToTick.push_back(playerBot);
		return playerBot.Controller;
	}

	static void SpawnBotsAtPlayerStarts(int AmountOfBots)
	{
		return;

		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		for (int i = 0; i < AmountOfBots; ++i)
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
		if (AllPlayerBotsToTick.size() == 0)
			return;

		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		// auto AllBuildingContainers = UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingContainer::StaticClass());

		// for (int i = 0; i < GameMode->GetAlivePlayers().Num(); ++i)
		for (auto& PlayerBot : AllPlayerBotsToTick)
		{
			auto CurrentPlayer = PlayerBot.Controller;

			if (CurrentPlayer->IsActorBeingDestroyed())
				continue;

			auto CurrentPawn = CurrentPlayer->GetPawn();

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
				static auto ServerThankBusDriverFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerControllerAthena.ServerThankBusDriver");
				CurrentPlayer->ProcessEvent(ServerThankBusDriverFn);
			}

			if (CurrentPawn)
			{
				if (PlayerBot.NextJumpTime <= UGameplayStatics::GetTimeSeconds(GetWorld()))
				{
					static auto JumpFn = FindObject<UFunction>(L"/Script/Engine.Character.Jump");

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

		// AllBuildingContainers.Free();
	}
}

namespace Bosses
{

}
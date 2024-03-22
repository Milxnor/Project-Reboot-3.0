#pragma once

#include "FortGameModeAthena.h"
#include "OnlineReplStructs.h"
#include "FortAthenaAIBotController.h"
#include "BuildingContainer.h"
#include "botnames.h"

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
	static inline UClass* PawnClass = nullptr;
	static inline UClass* ControllerClass = nullptr;

	AController* Controller = nullptr; // This can be 1. AFortAthenaAIBotController OR AFortPlayerControllerAthena
	bool bIsAthenaController = false;
	AFortPlayerPawnAthena* Pawn = nullptr;
	AFortPlayerStateAthena* PlayerState = nullptr;
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

	static bool ShouldUseAIBotController()
	{
		return Fortnite_Version >= 11 && Engine_Version < 500;
	}

	static void InitializeBotClasses()
	{
		static auto BlueprintGeneratedClassClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");

		if (!ShouldUseAIBotController())
		{
			PawnClass = FindObject<UClass>(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
			ControllerClass = AFortPlayerControllerAthena::StaticClass();
		}
		else
		{
			PawnClass = LoadObject<UClass>(L"/Game/Athena/AI/Phoebe/BP_PlayerPawn_Athena_Phoebe.BP_PlayerPawn_Athena_Phoebe_C", BlueprintGeneratedClassClass);
			// ControllerClass = PawnClass->CreateDefaultObject()->GetAIControllerClass();
		}

		if (/* !ControllerClass
			|| */ !PawnClass
			)
		{
			LOG_ERROR(LogBots, "Failed to find a class for the bots!");
			return;
		}
	}

	static bool IsReadyToSpawnBot()
	{
		return PawnClass;
	}

	void SetupInventory()
	{
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		if (!ShouldUseAIBotController()) // TODO REWRITE
		{
			AFortInventory** Inventory = nullptr;

			if (auto FortPlayerController = Cast<AFortPlayerController>(Controller))
			{
				Inventory = &FortPlayerController->GetWorldInventory();
			}
			else
			{
				if (auto FortAthenaAIBotController = Cast<AFortAthenaAIBotController>(Controller))
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

			static auto FortInventoryClass = FindObject<UClass>(L"/Script/FortniteGame.FortInventory"); // AFortInventory::StaticClass()
			*Inventory = GetWorld()->SpawnActor<AFortInventory>(FortInventoryClass, FTransform{}, CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AlwaysSpawn, false, Controller));

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
		}
	}

	void PickRandomLoadout()
	{
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
		PlayerState->Get(HeroTypeOffset) = HeroType;
	}

	void ApplyCosmeticLoadout()
	{
		static auto HeroTypeOffset = PlayerState->GetOffset("HeroType");
		const auto CurrentHeroType = PlayerState->Get(HeroTypeOffset);

		if (!CurrentHeroType)
		{
			LOG_WARN(LogBots, "CurrentHeroType called with an invalid HeroType!");
			return;
		}

		ApplyHID(Pawn, CurrentHeroType, true);
	}

	void SetName(const FString& NewName)
	{
		if (// true ||
			Fortnite_Version < 9
			)
		{
			if (auto PlayerController = Cast<APlayerController>(Controller))
			{
				PlayerController->ServerChangeName(NewName);
			}
		}
		else
		{
			auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
			GameMode->ChangeName(Controller, NewName, true);
		}

		PlayerState->OnRep_PlayerName(); // ?
	}

	FString GetRandomName() // Todo SetName(GetRandomName())
	{
		static int CurrentBotNum = 1;
		std::wstring BotNumWStr;
		FString NewName;

		if (Fortnite_Version < 9)
		{
			BotNumWStr = std::to_wstring(CurrentBotNum++);
			NewName = (L"RebootBot" + BotNumWStr).c_str();
		}
		else
		{
			if (Fortnite_Version < 11)
			{
				BotNumWStr = std::to_wstring(CurrentBotNum++ + 200);
				NewName = (std::format(L"Anonymous[{}]", BotNumWStr)).c_str();
			}
			else
			{
				if (!PlayerBotNames.empty())
				{
					// std::shuffle(PlayerBotNames.begin(), PlayerBotNames.end(), std::default_random_engine((unsigned int)time(0)));

					int RandomIndex = std::rand() % (PlayerBotNames.size() - 1);
					NewName = PlayerBotNames[RandomIndex];
					PlayerBotNames.erase(PlayerBotNames.begin() + RandomIndex);
				}
			}
		}

		return NewName;
	}

	void Initialize(const FTransform& SpawnTransform, AActor* InSpawnLocator)
	{
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		if (!IsReadyToSpawnBot())
		{
			LOG_ERROR(LogBots, "We are not prepared to spawn a bot!");
			return;
		}

		if (!ShouldUseAIBotController())
		{
			Controller = GetWorld()->SpawnActor<AController>(ControllerClass);
			Pawn = GetWorld()->SpawnActor<AFortPlayerPawnAthena>(PawnClass, SpawnTransform, CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));
			PlayerState = Cast<AFortPlayerStateAthena>(Controller->GetPlayerState());
		}
		else
		{
			Pawn = GameMode->GetServerBotManager()->GetCachedBotMutator()->SpawnBot(PawnClass, InSpawnLocator, SpawnTransform.Translation, SpawnTransform.Rotation.Rotator(), false);

			if (Fortnite_Version < 17)
				Controller = Cast<AFortAthenaAIBotController>(Pawn->GetController());
			else
				Controller = GetWorld()->SpawnActor<AFortAthenaAIBotController>(Pawn->GetAIControllerClass());

			PlayerState = Cast<AFortPlayerStateAthena>(Controller->GetPlayerState());
		}

		if (!Controller || !Pawn || !PlayerState)
		{
			LOG_ERROR(LogBots, "Failed to spawn controller, pawn or playerstate ({} {})!", bool(__int64(Controller)), bool(__int64(Pawn)), bool(__int64(Controller->GetPlayerState())));
			return;
		}

		PlayerState->SetIsBot(true);

		if (Controller->GetPawn() != Pawn)
		{
			Controller->Possess(Pawn);
		}

		FString BotNewName = GetRandomName();
		
		LOG_INFO(LogBots, "BotNewName: {}", BotNewName.ToString());
		SetName(BotNewName);

		PlayerState->GetTeamIndex() = GameMode->Athena_PickTeamHook(GameMode, 0, Controller);

		static auto SquadIdOffset = PlayerState->GetOffset("SquadId", false);

		if (SquadIdOffset != -1)
			PlayerState->GetSquadId() = PlayerState->GetTeamIndex() - NumToSubtractFromSquadId;

		GameState->AddPlayerStateToGameMemberInfo(PlayerState);

		Pawn->SetHealth(100);
		Pawn->SetMaxHealth(100);

		auto PlayerAbilitySet = GetPlayerAbilitySet();
		auto AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

		if (PlayerAbilitySet)
		{
			PlayerAbilitySet->GiveToAbilitySystem(AbilitySystemComponent);
		}

		SetupInventory();
		PickRandomLoadout();
		ApplyCosmeticLoadout();

		GameState->GetPlayersLeft()++;
		GameState->OnRep_PlayersLeft();

		if (auto FortPlayerControllerAthena = Cast<AFortPlayerControllerAthena>(Controller))
			GameMode->GetAlivePlayers().Add(FortPlayerControllerAthena);

		LOG_INFO(LogDev, "Finished spawning bot!")
	}
};

static inline std::vector<PlayerBot> AllPlayerBotsToTick;

namespace Bots
{
	static AController* SpawnBot(FTransform SpawnTransform, AActor* InSpawnLocator)
	{
		auto playerBot = PlayerBot();
		playerBot.Initialize(SpawnTransform, InSpawnLocator);
		AllPlayerBotsToTick.push_back(playerBot);
		return playerBot.Controller;
	}

	static void SpawnBotsAtPlayerStarts(int AmountOfBots)
	{
		return;

		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

		static auto FortPlayerStartCreativeClass = FindObject<UClass>(L"/Script/FortniteGame.FortPlayerStartCreative");
		static auto FortPlayerStartWarmupClass = FindObject<UClass>(L"/Script/FortniteGame.FortPlayerStartWarmup");
		TArray<AActor*> PlayerStarts = UGameplayStatics::GetAllActorsOfClass(GetWorld(), Globals::bCreative ? FortPlayerStartCreativeClass : FortPlayerStartWarmupClass);

		int ActorsNum = PlayerStarts.Num();

		// Actors.Free();

		if (ActorsNum == 0)
		{
			// LOG_INFO(LogDev, "No Actors!");
			return;
		}

		// Find playerstart (scuffed)

		for (int i = 0; i < AmountOfBots; ++i)
		{
			AActor* PlayerStart = PlayerStarts.at(std::rand() % (PlayerStarts.size() - 1));

			if (!PlayerStart)
			{
				return;
			}

			auto NewBot = SpawnBot(PlayerStart->GetTransform(), PlayerStart);
			NewBot->SetCanBeDamaged(Fortnite_Version < 7); // idk lol for spawn island
		}

		return;
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

			if (CurrentPawn->IsActorBeingDestroyed())
				continue;

			auto CurrentPlayerState = Cast<AFortPlayerStateAthena>(CurrentPlayer->GetPlayerState());

			if (!CurrentPlayerState 
				// || !CurrentPlayerState->IsBot()
				)
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

			if (PlayerBot.bIsAthenaController && CurrentPlayerState->IsInAircraft() && !CurrentPlayerState->HasThankedBusDriver())
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

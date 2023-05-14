#include "FortGameModeAthena.h"

#include "FortPlayerControllerAthena.h"
#include "FortPlaysetItemDefinition.h"
#include "FortAthenaCreativePortal.h"
#include "BuildingContainer.h"
#include "MegaStormManager.h"
#include "FortLootPackage.h"
#include "FortPlayerPawn.h"
#include "FortPickup.h"
#include "bots.h"

#include "FortAbilitySet.h"
#include "NetSerialization.h"
#include "GameplayStatics.h"
#include "DataTableFunctionLibrary.h"
#include "KismetStringLibrary.h"
#include "SoftObjectPtr.h"
#include "discord.h"
#include "BuildingGameplayActorSpawnMachine.h"

#include "vehicles.h"
#include "globals.h"
#include "events.h"
#include "reboot.h"
#include "ai.h"
#include "Map.h"
#include "OnlineReplStructs.h"
#include "BGA.h"
#include "vendingmachine.h"
#include "FortAthenaMutator.h"
#include "calendar.h"
#include "gui.h"
#include <random>

static UFortPlaylist* GetPlaylistToUse()
{
	// LOG_DEBUG(LogDev, "PlaylistName: {}", PlaylistName);

	auto Playlist = FindObject<UFortPlaylist>(PlaylistName);

	if (Globals::bGoingToPlayEvent)
	{
		if (Fortnite_Version != 12.61)
		{
			auto EventPlaylist = GetEventPlaylist();

			if (!EventPlaylist)
			{
				LOG_ERROR(LogPlaylist, "No event playlist! Turning off going to play event");
				Globals::bGoingToPlayEvent = false;
			}
			else
			{
				Playlist = EventPlaylist;
			}
		}
	}

	// SET OVERRIDE PLAYLIST DOWN HERE

	if (Globals::bCreative)
		Playlist = FindObject<UFortPlaylist>("/Game/Athena/Playlists/Creative/Playlist_PlaygroundV2.Playlist_PlaygroundV2");

	return Playlist;
}

enum class EFortAthenaPlaylist : uint8_t
{
	AthenaSolo = 0,
	AthenaDuo = 1,
	AthenaSquad = 2,
	EFortAthenaPlaylist_MAX = 3
};

EFortAthenaPlaylist GetPlaylistForOldVersion()
{
	return PlaylistName.contains("Solo") ? EFortAthenaPlaylist::AthenaSolo : PlaylistName.contains("Duo")
		? EFortAthenaPlaylist::AthenaDuo : PlaylistName.contains("Squad")
		? EFortAthenaPlaylist::AthenaSquad : EFortAthenaPlaylist::AthenaSolo;
}

FName AFortGameModeAthena::RedirectLootTier(const FName& LootTier)
{
	static auto RedirectAthenaLootTierGroupsOffset = this->GetOffset("RedirectAthenaLootTierGroups", false);

	if (RedirectAthenaLootTierGroupsOffset == -1)
	{
		static auto Loot_TreasureFName = UKismetStringLibrary::Conv_StringToName(L"Loot_Treasure");
		static auto Loot_AmmoFName = UKismetStringLibrary::Conv_StringToName(L"Loot_Ammo");

		if (LootTier == Loot_TreasureFName)
			return UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaTreasure");

		if (LootTier == Loot_AmmoFName)
			return UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaAmmoLarge");
		
		return LootTier;
	}

	auto& RedirectAthenaLootTierGroups = Get<TMap<FName, FName>>(RedirectAthenaLootTierGroupsOffset);

	for (auto& Pair : RedirectAthenaLootTierGroups)
	{
		auto& Key = Pair.Key();
		auto& Value = Pair.Value();

		// LOG_INFO(LogDev, "[{}] {} {}", i, Key.ComparisonIndex.Value ? Key.ToString() : "NULL", Key.ComparisonIndex.Value ? Value.ToString() : "NULL");

		if (Key == LootTier)
			return Value;
	}

	return LootTier;
}

UClass* AFortGameModeAthena::GetVehicleClassOverride(UClass* DefaultClass)
{
	static auto GetVehicleClassOverrideFn = FindObject<UFunction>("/Script/FortniteGame.FortGameModeAthena.GetVehicleClassOverride");

	if (!GetVehicleClassOverrideFn)
		return DefaultClass;

	struct { UClass* DefaultClass; UClass* ReturnValue; } GetVehicleClassOverride_Params{DefaultClass};
	
	this->ProcessEvent(GetVehicleClassOverrideFn, &GetVehicleClassOverride_Params);

	return GetVehicleClassOverride_Params.ReturnValue;
}

void AFortGameModeAthena::HandleSpawnRateForActorClass(UClass* ActorClass, float SpawnPercentage)
{
	TArray<AActor*> AllActors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass);

	int AmmoBoxesToDelete = std::round(AllActors.Num() - ((AllActors.Num()) * (SpawnPercentage / 100)));

	while (AmmoBoxesToDelete)
	{
		AllActors.at(rand() % AllActors.Num())->K2_DestroyActor();
		AmmoBoxesToDelete--;
	}
}

bool AFortGameModeAthena::Athena_ReadyToStartMatchHook(AFortGameModeAthena* GameMode)
{
	Globals::bHitReadyToStartMatch = true;

	auto GameState = GameMode->GetGameStateAthena();

	auto SetPlaylist = [&GameState, &GameMode](UObject* Playlist, bool bOnRep) -> void {
		if (Fortnite_Version >= 6.10)
		{
			auto CurrentPlaylistInfo = GameState->GetPtr<FFastArraySerializer>("CurrentPlaylistInfo");

			static auto PlaylistReplicationKeyOffset = FindOffsetStruct("/Script/FortniteGame.PlaylistPropertyArray", "PlaylistReplicationKey");
			static auto BasePlaylistOffset = FindOffsetStruct("/Script/FortniteGame.PlaylistPropertyArray", "BasePlaylist");
			static auto OverridePlaylistOffset = FindOffsetStruct("/Script/FortniteGame.PlaylistPropertyArray", "OverridePlaylist");

			*(UObject**)(__int64(CurrentPlaylistInfo) + BasePlaylistOffset) = Playlist;
			*(UObject**)(__int64(CurrentPlaylistInfo) + OverridePlaylistOffset) = Playlist;

			(*(int*)(__int64(CurrentPlaylistInfo) + PlaylistReplicationKeyOffset))++;
			CurrentPlaylistInfo->MarkArrayDirty();

			auto aeuh = *(UObject**)(__int64(CurrentPlaylistInfo) + BasePlaylistOffset);

			if (aeuh)
			{
				GameMode->SetCurrentPlaylistName(aeuh);

				/* if (Fortnite_Version >= 13)
				{
					static auto LastSafeZoneIndexOffset = aeuh->GetOffset("LastSafeZoneIndex");

					if (LastSafeZoneIndexOffset != -1)
					{
						*(int*)(__int64(aeuh) + LastSafeZoneIndexOffset) = 0;
					}
				} */
			}
		}
		else
		{
			static auto CurrentPlaylistDataOffset = GameState->GetOffset("CurrentPlaylistData", false);

			if (CurrentPlaylistDataOffset != -1)
				GameState->Get(CurrentPlaylistDataOffset) = Playlist;
		}

		if (bOnRep)
			GameState->OnRep_CurrentPlaylistInfo();
	};

	/* auto& LocalPlayers = GetLocalPlayers();

	if (LocalPlayers.Num() && LocalPlayers.Data)
	{
		LocalPlayers.Remove(0);
	} */

	static int LastNum2 = 1;

	if (AmountOfRestarts != LastNum2)
	{
		LastNum2 = AmountOfRestarts;

		LOG_INFO(LogDev, "Presetup!");

		static auto WarmupRequiredPlayerCountOffset = GameMode->GetOffset("WarmupRequiredPlayerCount");
		GameMode->Get<int>(WarmupRequiredPlayerCountOffset) = 1;

		static auto CurrentPlaylistDataOffset = GameState->GetOffset("CurrentPlaylistData", false);

		if (CurrentPlaylistDataOffset != -1 || Fortnite_Version >= 6) // idk when they switched off id
		{
			auto PlaylistToUse = GetPlaylistToUse();

			if (!PlaylistToUse)
			{
				LOG_ERROR(LogPlaylist, "Failed to find playlist! Proceeding, but will probably not work as expected!");
			}
			else
			{
				if (Fortnite_Version >= 4)
				{
					SetPlaylist(PlaylistToUse, true);

					auto CurrentPlaylist = GameState->GetCurrentPlaylist();
					LOG_INFO(LogDev, "Set playlist to {}!", CurrentPlaylist->IsValidLowLevel() ? CurrentPlaylist->GetFullName() : "Invalid");
				}
			}
		}
		else
		{
			auto OldPlaylist = GetPlaylistForOldVersion();
		}

		auto Fortnite_Season = std::floor(Fortnite_Version);

		// if (false) // Manual foundation showing
		{
			if (Fortnite_Season >= 7 && Fortnite_Season <= 10)
			{
				if (Fortnite_Season == 7)
				{
					ShowFoundation(FindObject<AActor>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_POI_25x36")); // Polar Peak
					ShowFoundation(FindObject<AActor>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.ShopsNew")); // Tilted Tower Shops, is this 7.40 specific?
				}

				else if (Fortnite_Season == 8)
				{
					auto Volcano = FindObject<AActor>(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_POI_50x53_Volcano"));
					ShowFoundation(Volcano);
				}

				else if (Fortnite_Season == 10)
				{
					if (Fortnite_Version >= 10.20)
					{
						auto Island = FindObject<AActor>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_StreamingTest16");
						ShowFoundation(Island);
					}
				}
			}

			if (Fortnite_Version == 17.50) {
				auto FarmAfter = FindObject<AActor>(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.farmbase_2"));
				ShowFoundation(FarmAfter);

				auto FarmPhase = FindObject<AActor>(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.Farm_Phase_03")); // Farm Phases (Farm_Phase_01, Farm_Phase_02 and Farm_Phase_03)
				ShowFoundation(FarmPhase);
			}

			if (Fortnite_Version == 17.40) {
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.CoralPhase_02")); // Coral Castle Phases (CoralPhase_01, CoralPhase_02 and CoralPhase_03)
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation_0")); // CoralFoundation_01
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation6")); // CoralFoundation_05
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation3")); // CoralFoundation_07
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation2_1")); // CoralFoundation_10
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation4"));
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation5"));
			}

			if (Fortnite_Version == 17.30) {
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.Slurpy_Phase03")); // There are 1, 2 and 3
			}

			if (Fortnite_Season == 13)
			{
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.Lobby_Foundation"));

				// SpawnIsland->RepData->Soemthing = FoundationSetup->LobbyLocation;
			}

			if (Fortnite_Version == 12.41)
			{
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_Athena_POI_19x19_2"));
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head6_18"));
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head5_14"));
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head3_8"));
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head_2"));
				ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head4_11"));
			}

			if (Fortnite_Version == 11.31)
			{
				// There are also christmas trees & stuff but we need to find a better way to stream that because there's a lot.

				if (false) // If the client loads this, it says the package doesnt exist...
				{
					ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_5x5_Galileo_Ferry_1"));
					ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_5x5_Galileo_Ferry_2"));
					ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_5x5_Galileo_Ferry_3"));
					ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_5x5_Galileo_Ferry_4"));
					ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_5x5_Galileo_Ferry_5"));
				}
			}

			GET_PLAYLIST(GameState);

			if (CurrentPlaylist)
			{
				static auto AdditionalLevelsOffset = CurrentPlaylist->GetOffset("AdditionalLevels", false);

				if (AdditionalLevelsOffset != -1)
				{
					auto& AdditionalLevels = CurrentPlaylist->Get<TArray<TSoftObjectPtr<UClass>>>(AdditionalLevelsOffset);

					LOG_INFO(LogPlaylist, "Loading {} playlist levels.", AdditionalLevels.Num());

					for (int i = 0; i < AdditionalLevels.Num(); i++)
					{
						// auto World = Cast<UWorld>(Playlist->AdditionalLevels[i].Get());
						// StreamLevel(UKismetSystemLibrary::GetPathName(World->PersistentLevel).ToString());
						auto LevelName = AdditionalLevels.at(i).SoftObjectPtr.ObjectID.AssetPathName.ToString();
						LOG_INFO(LogPlaylist, "Loading level {}.", LevelName);
						StreamLevel(LevelName);
					}
				}
			}
		}

		if (Fortnite_Version == 7.30)
		{
			if (true) // idfk if the stage only showed on marshmello playlist
			{
				auto PleasantParkIdk = FindObject<AActor>(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.PleasentParkFestivus"));
				ShowFoundation(PleasantParkIdk);
			}
			else
			{
				auto PleasantParkGround = FindObject<AActor>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.PleasentParkDefault");
				ShowFoundation(PleasantParkGround);
			}
		}

		if (Fortnite_Season == 6)
		{
			if (Fortnite_Version != 6.10)
			{
				auto Lake = FindObject<AActor>(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Lake1"));
				auto Lake2 = FindObject<AActor>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Lake2");

				Fortnite_Version <= 6.21 ? ShowFoundation(Lake) : ShowFoundation(Lake2);
			}
			else
			{
				auto Lake = FindObject<AActor>(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_StreamingTest12"));
				ShowFoundation(Lake);
			}

			auto FloatingIsland = Fortnite_Version == 6.10 ? FindObject<AActor>(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_StreamingTest13")) :
				FindObject<AActor>(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_FloatingIsland"));

			ShowFoundation(FloatingIsland);

			UObject* Scripting = FindObject<AActor>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.BP_IslandScripting3"); // bruh

			if (Scripting)
			{
				static auto UpdateMapOffset = Scripting->GetOffset("UpdateMap", false);

				if (UpdateMapOffset != -1)
				{
					Scripting->Get<bool>(UpdateMapOffset) = true;

					static auto OnRep_UpdateMap = FindObject<UFunction>("/Game/Athena/Prototype/Blueprints/Island/BP_IslandScripting.BP_IslandScripting_C.OnRep_UpdateMap");
					Scripting->ProcessEvent(OnRep_UpdateMap);
				}
			}
		}

		if (Fortnite_Version == 14.60 && Globals::bGoingToPlayEvent)
		{
			ShowFoundation(FindObject<AActor>("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.Lobby_Foundation3")); // Aircraft Carrier
		}

		auto TheBlock = FindObject<AActor>("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.SLAB_2"); // SLAB_3 is blank

		if (TheBlock)
			ShowFoundation(TheBlock);

		static auto GameState_AirCraftBehaviorOffset = GameState->GetOffset("AirCraftBehavior", false);

		if (GameState_AirCraftBehaviorOffset != -1)
		{
			GET_PLAYLIST(GameState);

			if (CurrentPlaylist)
			{
				static auto Playlist_AirCraftBehaviorOffset = CurrentPlaylist->GetOffset("AirCraftBehavior", false);

				if (Playlist_AirCraftBehaviorOffset != -1)
				{
					GameState->Get<uint8_t>(GameState_AirCraftBehaviorOffset) = CurrentPlaylist->Get<uint8_t>(Playlist_AirCraftBehaviorOffset);
				}
			}
		}

		static auto bWorldIsReadyOffset = GameMode->GetOffset("bWorldIsReady");
		SetBitfield(GameMode->GetPtr<PlaceholderBitfield>(bWorldIsReadyOffset), 1, true); // idk when we actually set this

		// Calendar::SetSnow(1000);

		Globals::bInitializedPlaylist = true;
	}

	static int LastNum6 = 1;

	if (AmountOfRestarts != LastNum6)
	{
		LastNum6 = AmountOfRestarts;

		if (Globals::bGoingToPlayEvent && DoesEventRequireLoading())
		{
			bool bb;
			LoadEvent(&bb);

			if (!bb)
				LastNum6 = -1;
		}
	}

	static int LastNum5 = 1;

	if (AmountOfRestarts != LastNum5 && LastNum6 == AmountOfRestarts) // Make sure we loaded the event.
	{
		LastNum5 = AmountOfRestarts;

		if (Globals::bGoingToPlayEvent)
		{
			bool bb;
			CallOnReadys(&bb);

			if (!bb)
				LastNum5 = -1;
		}
	}

	// if (!Globals::bCreative)
	{
		static auto FortPlayerStartCreativeClass = FindObject<UClass>(L"/Script/FortniteGame.FortPlayerStartCreative");
		static auto FortPlayerStartWarmupClass = FindObject<UClass>(L"/Script/FortniteGame.FortPlayerStartWarmup");
		TArray<AActor*> Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), Globals::bCreative ? FortPlayerStartCreativeClass : FortPlayerStartWarmupClass);

		int ActorsNum = Actors.Num();

		Actors.Free();

		if (ActorsNum == 0)
			return false;
	}

	auto MapInfo = GameState->GetMapInfo();
	
	if (!MapInfo && Engine_Version >= 421)
		return false;

	static int LastNum = 1;

	if (AmountOfRestarts != LastNum)
	{
		LastNum = AmountOfRestarts;

		LOG_INFO(LogDev, "Initializing!");

		if (std::floor(Fortnite_Version) == 3)
			SetPlaylist(GetPlaylistToUse(), true);

		LOG_INFO(LogDev, "GameMode 0x{:x}", __int64(GameMode));

		bool bShouldSkipAircraft = false;

		GET_PLAYLIST(GameState);

		if (CurrentPlaylist)
		{
			static auto bSkipAircraftOffset = CurrentPlaylist->GetOffset("bSkipAircraft", false);

			if (bSkipAircraftOffset != -1)
				bShouldSkipAircraft = CurrentPlaylist->Get<bool>(bSkipAircraftOffset);
		}

		float Duration = bShouldSkipAircraft ? 0 : 100000;
		float EarlyDuration = Duration;

		float TimeSeconds = UGameplayStatics::GetTimeSeconds(GetWorld());

		static auto WarmupCountdownEndTimeOffset = GameState->GetOffset("WarmupCountdownEndTime");
		static auto WarmupCountdownStartTimeOffset = GameState->GetOffset("WarmupCountdownStartTime");
		static auto WarmupCountdownDurationOffset = GameMode->GetOffset("WarmupCountdownDuration");
		static auto WarmupEarlyCountdownDurationOffset = GameMode->GetOffset("WarmupEarlyCountdownDuration");

		GameState->Get<float>(WarmupCountdownEndTimeOffset) = TimeSeconds + Duration;
		GameMode->Get<float>(WarmupCountdownDurationOffset) = Duration;

		GameState->Get<float>(WarmupCountdownStartTimeOffset) = TimeSeconds;
		GameMode->Get<float>(WarmupEarlyCountdownDurationOffset) = EarlyDuration;

		static auto GameSessionOffset = GameMode->GetOffset("GameSession");
		auto GameSession = GameMode->Get<AActor*>(GameSessionOffset);
		static auto MaxPlayersOffset = GameSession->GetOffset("MaxPlayers");

		GameSession->Get<int>(MaxPlayersOffset) = 100;

		GameState->OnRep_CurrentPlaylistInfo(); // ?

		// SetupNavConfig();

		static auto bAlwaysDBNOOffset = GameMode->GetOffset("bAlwaysDBNO");
		// GameMode->Get<bool>(bAlwaysDBNOOffset) = true;
		
		static auto GameState_AirCraftBehaviorOffset = GameState->GetOffset("AirCraftBehavior", false);

		if (GameState_AirCraftBehaviorOffset != -1)
		{
			if (CurrentPlaylist)
			{
				static auto Playlist_AirCraftBehaviorOffset = CurrentPlaylist->GetOffset("AirCraftBehavior", false);

				if (Playlist_AirCraftBehaviorOffset != -1)
				{
					GameState->Get<uint8_t>(GameState_AirCraftBehaviorOffset) = CurrentPlaylist->Get<uint8_t>(Playlist_AirCraftBehaviorOffset);
				}
			}
		}

		LOG_INFO(LogDev, "Initialized!");
	}

	static auto TeamsOffset = GameState->GetOffset("Teams");
	auto& Teams = GameState->Get<TArray<UObject*>>(TeamsOffset);

	if (Teams.Num() <= 0)
		return false;

	static int LastNum3 = 1;

	if (AmountOfRestarts != LastNum3)
	{
		LastNum3 = AmountOfRestarts;
		++Globals::AmountOfListens;

		LOG_INFO(LogNet, "Attempting to listen!");

		GetWorld()->Listen();

		SetupAIDirector();
		SetupServerBotManager();

		if (auto TeamsArrayContainer = GameState->GetTeamsArrayContainer())
		{
			TeamsArrayContainer->TeamIndexesArray.Free();

			for (int i = 0; i < 100; i++)
			{
				TeamsArrayContainer->TeamIndexesArray.Add(INT_MAX); // Bro what
			}

			TeamsArrayContainer->SquadIdsArray.Free();

			for (int i = 0; i < 100; i++)
			{
				TeamsArrayContainer->SquadIdsArray.Add(INT_MAX); // Bro what
			}

			// We aren't "freeing", it's just not zero'd I guess?

			for (int i = 0; i < TeamsArrayContainer->TeamsArray.Num(); i++)
			{
				TeamsArrayContainer->TeamsArray.at(i).Free();
			}

			for (int i = 0; i < TeamsArrayContainer->SquadsArray.Num(); i++)
			{
				TeamsArrayContainer->SquadsArray.at(i).Free();
			}
		}

		auto AllRebootVans = UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingGameplayActorSpawnMachine::StaticClass());

		for (int i = 0; i < AllRebootVans.Num(); i++)
		{
			auto CurrentRebootVan = (ABuildingGameplayActorSpawnMachine*)AllRebootVans.at(i);
			static auto FortPlayerStartClass = FindObject<UClass>(L"/Script/FortniteGame.FortPlayerStart");
			CurrentRebootVan->GetResurrectLocation() = CurrentRebootVan->GetClosestActor(FortPlayerStartClass, 100);
		}

		AllRebootVans.Free();

		static auto DefaultRebootMachineHotfixOffset = GameState->GetOffset("DefaultRebootMachineHotfix", false);

		if (DefaultRebootMachineHotfixOffset != -1)
		{
			// LOG_INFO(LogDev, "Beraau: {}", GameState->Get<float>(DefaultRebootMachineHotfixOffset));
			GameState->Get<float>(DefaultRebootMachineHotfixOffset) = 1; // idk i dont think we need to set
		}

		if (AmountOfBotsToSpawn != 0)
		{
			Bots::SpawnBotsAtPlayerStarts(AmountOfBotsToSpawn);
		}

		UptimeWebHook.send_message(std::format("Server up! {} {}", Fortnite_Version, PlaylistName)); // PlaylistName sometimes isn't always what we use!

		if (std::floor(Fortnite_Version) == 5)
		{
			auto NewFn = FindObject<UFunction>(L"/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C.New");

			if (NewFn && (Fortnite_Version == 5.30 ? !Globals::bGoingToPlayEvent : true))
			{
				auto Loader = GetEventLoader("/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C");

				LOG_INFO(LogDev, "Loader: {}", __int64(Loader));

				if (Loader)
				{
					int32 NewParam = 1;
					// Loader->ProcessEvent(NextFn, &NewParam);
					Loader->ProcessEvent(NewFn, &NewParam);
				}
			}
		}

		std::vector<std::string> WorldNamesToStreamAllFoundationsIn; // wtf

		if (Fortnite_Version == 6.21)
		{
			WorldNamesToStreamAllFoundationsIn.push_back("/Temp/Game/Athena/Maps/POI/Athena_POI_Lake_002_5d9a86c8.Athena_POI_Lake_002:PersistentLevel.");
		}

		if (Fortnite_Version == 7.30)
		{
			// idk what one we actually need
			WorldNamesToStreamAllFoundationsIn.push_back("/Temp/Game/Athena/Maps/POI/Athena_POI_CommunityPark_003_77acf920");
			WorldNamesToStreamAllFoundationsIn.push_back("/Temp/Game/Athena/Maps/POI/Athena_POI_CommunityPark_003_M_5c711338");
		}

		if (WorldNamesToStreamAllFoundationsIn.size() > 0)
		{
			auto ObjectNum = ChunkedObjects ? ChunkedObjects->Num() : UnchunkedObjects ? UnchunkedObjects->Num() : 0;

			for (int i = 0; i < ObjectNum; i++)
			{
				auto CurrentObject = GetObjectByIndex(i);

				if (!CurrentObject)
					continue;

				static auto BuildingFoundationClass = FindObject<UClass>(L"/Script/FortniteGame.BuildingFoundation");

				if (!CurrentObject->IsA(BuildingFoundationClass))
					continue;

				auto CurrentObjectFullName = CurrentObject->GetFullName(); // We can do GetPathName() and starts with but eh.

				for (int z = 0; z < WorldNamesToStreamAllFoundationsIn.size(); z++)
				{
					if (CurrentObject->GetFullName().contains(WorldNamesToStreamAllFoundationsIn.at(z)))
					{
						// I think we only have to set bServerStreamedInLevel.
						ShowFoundation((AActor*)CurrentObject);
						continue;
					}
				}
			}
		}

		Globals::bStartedListening = true;
	}

	bool Ret = false;

	if (Engine_Version >= 424) // returning true is stripped on c2+
	{
		static auto WarmupRequiredPlayerCountOffset = GameMode->GetOffset("WarmupRequiredPlayerCount");

		if (GameState->GetPlayersLeft() >= GameMode->Get<int>(WarmupRequiredPlayerCountOffset))
		{
			if (MapInfo)
			{
				static auto FlightInfosOffset = MapInfo->GetOffset("FlightInfos");

				// if (MapInfo->Get<TArray<__int64>>(FlightInfosOffset).ArrayNum > 0)
				{
					// LOG_INFO(LogDev, "ReadyToStartMatch Return Address: 0x{:x}", __int64(_ReturnAddress()) - __int64(GetModuleHandleW(0)));
					Ret = true;
				}
			}
		}
	}

	if (!Ret)
		Ret = Athena_ReadyToStartMatchOriginal(GameMode);

	if (Ret)
	{
		// We are assuming it successfully became warmup.

		std::vector<std::pair<AFortAthenaMutator*, UFunction*>> FunctionsToCall;

		LoopMutators([&](AFortAthenaMutator* Mutator) { LOG_INFO(LogGame, "Mutator {}", Mutator->GetPathName()); });
		LoopMutators([&](AFortAthenaMutator* Mutator) { FunctionsToCall.push_back(std::make_pair(Mutator, Mutator->FindFunction("OnGamePhaseStepChanged"))); });

		static int LastNum1 = 3125;

		if (LastNum1 != Globals::AmountOfListens)
		{
			LastNum1 = Globals::AmountOfListens;

			for (auto& FunctionToCallPair : FunctionsToCall)
			{
				// On newer versions there is a second param.

				// LOG_INFO(LogDev, "FunctionToCallPair.second: {}", __int64(FunctionToCallPair.second));

				if (FunctionToCallPair.second)
				{
					if (Fortnite_Version < 10)
					{
						// mem leak btw

						auto a = ConstructOnGamePhaseStepChangedParams(EAthenaGamePhaseStep::None);

						if (a)
						{
							FunctionToCallPair.first->ProcessEvent(FunctionToCallPair.second, a);
							FunctionToCallPair.first->ProcessEvent(FunctionToCallPair.second, ConstructOnGamePhaseStepChangedParams(EAthenaGamePhaseStep::Setup));
							FunctionToCallPair.first->ProcessEvent(FunctionToCallPair.second, ConstructOnGamePhaseStepChangedParams(EAthenaGamePhaseStep::Warmup));
						}
					}
				}
			}
		}
	}

	return Ret;
}

int AFortGameModeAthena::Athena_PickTeamHook(AFortGameModeAthena* GameMode, uint8 preferredTeam, AActor* Controller)
{
	bool bIsBot = false;

	auto PlayerState = ((APlayerController*)Controller)->GetPlayerState();

	if (!PlayerState)
	{
		LOG_ERROR(LogGame, "Player has no playerstate!");
		return 0;
	}

	if (PlayerState)
	{
		bIsBot = PlayerState->IsBot();
	}

	// VERY BASIC IMPLEMENTATION

	// LOG_INFO(LogTeams, "PickTeam called!");

	auto GameState = Cast<AFortGameStateAthena>(GameMode->GetGameState());

	static auto CurrentPlaylistDataOffset = GameState->GetOffset("CurrentPlaylistData", false);

	UFortPlaylist* Playlist = nullptr;

	bool bVersionHasPlaylist = false;

	if (CurrentPlaylistDataOffset != -1 || Fortnite_Version >= 6)
	{
		bVersionHasPlaylist = true;
		Playlist = CurrentPlaylistDataOffset == -1 && Fortnite_Version < 6 ? nullptr : GameState->GetCurrentPlaylist();
	}

	static int DefaultFirstTeam = 3;

	bool bShouldSpreadTeams = false;

	if (Playlist)
	{
		static auto bIsLargeTeamGameOffset = Playlist->GetOffset("bIsLargeTeamGame");
		bool bIsLargeTeamGame = Playlist->Get<bool>(bIsLargeTeamGameOffset);
		bShouldSpreadTeams = bIsLargeTeamGame;
	}

	static int CurrentTeamMembers = 0; // bad
	static int Current = DefaultFirstTeam;
	static int NextTeamIndex = DefaultFirstTeam;

	static int LastNum = 1;

	if (Globals::AmountOfListens != LastNum)
	{
		LastNum = Globals::AmountOfListens;

		Current = DefaultFirstTeam;
		NextTeamIndex = DefaultFirstTeam;
		CurrentTeamMembers = 0;
	}

	// std::cout << "Dru!\n";

	int MaxSquadSize = 1;
	int TeamsNum = 0;

	if (bVersionHasPlaylist)
	{
		if (!Playlist)
		{
			CurrentTeamMembers = 0;
			// LOG_INFO(LogTeams, "Player is going on team {} with {} members (No Playlist).", Current, CurrentTeamMembers);
			CurrentTeamMembers++;
			return Current++;
		}

		static auto MaxSquadSizeOffset = Playlist->GetOffset("MaxSquadSize");
		MaxSquadSize = Playlist->Get<int>(MaxSquadSizeOffset);

		static auto bShouldSpreadTeamsOffset = Playlist->GetOffset("bShouldSpreadTeams", false);

		if (bShouldSpreadTeamsOffset != -1)
		{
			// bShouldSpreadTeams = Playlist->Get<bool>(bShouldSpreadTeamsOffset);
		}

		static auto MaxTeamCountOffset = Playlist->GetOffset("MaxTeamCount", false);

		if (MaxTeamCountOffset != -1)
			TeamsNum = Playlist->Get<int>(MaxTeamCountOffset);
	}
	else
	{
		auto OldPlaylist = GetPlaylistForOldVersion();
		MaxSquadSize = OldPlaylist == EFortAthenaPlaylist::AthenaSolo ? 1
			: OldPlaylist == EFortAthenaPlaylist::AthenaDuo ? 2
			: OldPlaylist == EFortAthenaPlaylist::AthenaSquad ? 4
			: 1;

		TeamsNum = 100;
	}

	// LOG_INFO(LogTeams, "Before team assigning NextTeamIndex: {} CurrentTeamMembers: {}", NextTeamIndex, CurrentTeamMembers);

	if (!bShouldSpreadTeams)
	{
		if (CurrentTeamMembers >= MaxSquadSize)
		{
			NextTeamIndex++;
			CurrentTeamMembers = 0;
		}
	}
	else
	{
		// Basically, this goes through all the teams, and whenever we hit the last team, we go back to the first.

		auto Idx = NextTeamIndex - 2; // 1-100

		if (CurrentTeamMembers >= 1) // We spread every player.
		{
			if (Idx >= TeamsNum)
				NextTeamIndex = DefaultFirstTeam;
			else
				NextTeamIndex++;

			CurrentTeamMembers = 0;
		}
	}

	LOG_INFO(LogTeams, "Spreading Teams {} [{}] Player is going on team {} with {} members.", bShouldSpreadTeams, TeamsNum, NextTeamIndex, CurrentTeamMembers);

	CurrentTeamMembers++;

	TWeakObjectPtr<AFortPlayerStateAthena> WeakPlayerState{};
	WeakPlayerState.ObjectIndex = PlayerState->InternalIndex;
	WeakPlayerState.ObjectSerialNumber = GetItemByIndex(PlayerState->InternalIndex)->SerialNumber;

	if (auto TeamsArrayContainer = GameState->GetTeamsArrayContainer())
	{
		auto& TeamArray = TeamsArrayContainer->TeamsArray.at(NextTeamIndex);
		LOG_INFO(LogDev, "TeamsArrayContainer->TeamsArray.Num(): {}", TeamsArrayContainer->TeamsArray.Num());
		LOG_INFO(LogDev, "TeamArray.Num(): {}", TeamArray.Num());

		TeamArray.Add(WeakPlayerState);
	}

	return NextTeamIndex;
}

void AFortGameModeAthena::Athena_HandleStartingNewPlayerHook(AFortGameModeAthena* GameMode, AActor* NewPlayerActor)
{
	if (NewPlayerActor == GetLocalPlayerController()) // we dont really need this but it also functions as a nullptr check usually
		return;

	auto GameState = GameMode->GetGameStateAthena();

	static auto CurrentPlaylistDataOffset = GameState->GetOffset("CurrentPlaylistData", false);
	auto CurrentPlaylist = CurrentPlaylistDataOffset == -1 && Fortnite_Version < 6 ? nullptr : GameState->GetCurrentPlaylist();

	LOG_INFO(LogPlayer, "HandleStartingNewPlayer!");

	if (Globals::bAutoRestart)
	{
		static int LastNum123 = 15;

		if (GetWorld()->GetNetDriver()->GetClientConnections().Num() >= NumRequiredPlayersToStart && LastNum123 != Globals::AmountOfListens)
		{
			LastNum123 = Globals::AmountOfListens;

			float Duration = AutoBusStartSeconds;
			float EarlyDuration = Duration;

			float TimeSeconds = UGameplayStatics::GetTimeSeconds(GetWorld());

			static auto WarmupCountdownEndTimeOffset = GameState->GetOffset("WarmupCountdownEndTime");
			static auto WarmupCountdownStartTimeOffset = GameState->GetOffset("WarmupCountdownStartTime");
			static auto WarmupCountdownDurationOffset = GameMode->GetOffset("WarmupCountdownDuration");
			static auto WarmupEarlyCountdownDurationOffset = GameMode->GetOffset("WarmupEarlyCountdownDuration");

			GameState->Get<float>(WarmupCountdownEndTimeOffset) = TimeSeconds + Duration;
			GameMode->Get<float>(WarmupCountdownDurationOffset) = Duration;

			GameState->Get<float>(WarmupCountdownStartTimeOffset) = TimeSeconds;
			GameMode->Get<float>(WarmupEarlyCountdownDurationOffset) = EarlyDuration;

			LOG_INFO(LogDev, "Auto starting bus in {}.", AutoBusStartSeconds);
		}
	}

	// if (Engine_Version < 427)
	{
		static int LastNum69 = 19451;

		if (LastNum69 != Globals::AmountOfListens)
		{
			LastNum69 = Globals::AmountOfListens;

			// is there spawn percentage for vending machines?

			bool bShouldDestroyVendingMachines = Fortnite_Version < 3.4 || Engine_Version >= 424;

			if (!bShouldDestroyVendingMachines)
			{
				if (Globals::bFillVendingMachines)
					FillVendingMachines();
			}
			else
			{
				auto VendingMachineClass = FindObject<UClass>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C");
				auto AllVendingMachines = UGameplayStatics::GetAllActorsOfClass(GetWorld(), VendingMachineClass);

				for (int i = 0; i < AllVendingMachines.Num(); i++)
				{
					auto VendingMachine = (ABuildingItemCollectorActor*)AllVendingMachines.at(i);

					if (!VendingMachine)
						continue;

					VendingMachine->K2_DestroyActor();
				}

				AllVendingMachines.Free();
			}

			if (Fortnite_Version < 19) // fr idk what i did too lazy to debug
				SpawnBGAs();

			// Handle spawn rate

			auto MapInfo = GameState->GetMapInfo();

			if (MapInfo)
			{
				if (Fortnite_Version >= 3.3)
				{
					MapInfo->SpawnLlamas();
				}

				if (false)
				{
					float AmmoBoxMinSpawnPercent = UDataTableFunctionLibrary::EvaluateCurveTableRow(
						MapInfo->GetAmmoBoxMinSpawnPercent()->GetCurve().CurveTable, MapInfo->GetAmmoBoxMinSpawnPercent()->GetCurve().RowName, 0
					);

					float AmmoBoxMaxSpawnPercent = UDataTableFunctionLibrary::EvaluateCurveTableRow(
						MapInfo->GetAmmoBoxMaxSpawnPercent()->GetCurve().CurveTable, MapInfo->GetAmmoBoxMaxSpawnPercent()->GetCurve().RowName, 0
					);

					LOG_INFO(LogDev, "AmmoBoxMinSpawnPercent: {} AmmoBoxMaxSpawnPercent: {}", AmmoBoxMinSpawnPercent, AmmoBoxMaxSpawnPercent);

					std::random_device AmmoBoxRd;
					std::mt19937 AmmoBoxGen(AmmoBoxRd());
					std::uniform_int_distribution<> AmmoBoxDis(AmmoBoxMinSpawnPercent * 100, AmmoBoxMaxSpawnPercent * 100 + 1); // + 1 ?

					float AmmoBoxSpawnPercent = AmmoBoxDis(AmmoBoxGen);

					HandleSpawnRateForActorClass(MapInfo->GetAmmoBoxClass(), AmmoBoxSpawnPercent);
				}
			}

			auto SpawnIsland_FloorLoot = FindObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");
			auto BRIsland_FloorLoot = FindObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");

			TArray<AActor*> SpawnIsland_FloorLoot_Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnIsland_FloorLoot);
			TArray<AActor*> BRIsland_FloorLoot_Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), BRIsland_FloorLoot);

			auto SpawnIslandTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaFloorLoot_Warmup");
			auto BRIslandTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaFloorLoot");

			float UpZ = 50;

			uint8 SpawnFlag = EFortPickupSourceTypeFlag::GetContainerValue();

			bool bTest = false;
			bool bPrintWarmup = false;

			for (int i = 0; i < SpawnIsland_FloorLoot_Actors.Num(); i++)
			{
				ABuildingContainer* CurrentActor = (ABuildingContainer*)SpawnIsland_FloorLoot_Actors.at(i);
				auto Location = CurrentActor->GetActorLocation();
				Location.Z += UpZ;

				std::vector<LootDrop> LootDrops = PickLootDrops(SpawnIslandTierGroup, -1, bPrintWarmup);

				for (auto& LootDrop : LootDrops)
				{
					PickupCreateData CreateData;
					CreateData.bToss = true;
					CreateData.ItemEntry = FFortItemEntry::MakeItemEntry(LootDrop->GetItemDefinition(), LootDrop->GetCount(), LootDrop->GetLoadedAmmo());
					CreateData.SpawnLocation = Location;
					CreateData.SourceType = SpawnFlag;
					CreateData.bRandomRotation = true;
					CreateData.bShouldFreeItemEntryWhenDeconstructed = true;

					auto Pickup = AFortPickup::SpawnPickup(CreateData);
				}

				if (!bTest)
					CurrentActor->K2_DestroyActor();
			}

			bool bPrint = false;

			int spawned = 0;

			for (int i = 0; i < BRIsland_FloorLoot_Actors.Num(); i++)
			{
				ABuildingContainer* CurrentActor = (ABuildingContainer*)BRIsland_FloorLoot_Actors.at(i);
				spawned++;
				auto Location = CurrentActor->GetActorLocation();
				Location.Z += UpZ;

				std::vector<LootDrop> LootDrops = PickLootDrops(BRIslandTierGroup, -1, bPrint);

				for (auto& LootDrop : LootDrops)
				{
					PickupCreateData CreateData;
					CreateData.bToss = true;
					CreateData.ItemEntry = FFortItemEntry::MakeItemEntry(LootDrop->GetItemDefinition(), LootDrop->GetCount(), LootDrop->GetLoadedAmmo());
					CreateData.SpawnLocation = Location;
					CreateData.SourceType = SpawnFlag;
					CreateData.bRandomRotation = true;
					CreateData.bShouldFreeItemEntryWhenDeconstructed = true;

					auto Pickup = AFortPickup::SpawnPickup(CreateData);
				}

				if (!bTest)
					CurrentActor->K2_DestroyActor();
			}

			SpawnIsland_FloorLoot_Actors.Free();
			BRIsland_FloorLoot_Actors.Free();

			LOG_INFO(LogDev, "Spawned loot!");
		}
	}

	if (Engine_Version >= 423 && Fortnite_Version <= 12.61) // 423+ we need to spawn manually and vehicle sync doesn't work on >S13.
	{
		static int LastNum420 = 114;

		if (LastNum420 != Globals::AmountOfListens)
		{
			LastNum420 = Globals::AmountOfListens;

			SpawnVehicles2();
		}
	}

	auto NewPlayer = (AFortPlayerControllerAthena*)NewPlayerActor;

	auto PlayerStateAthena = NewPlayer->GetPlayerStateAthena();

	if (!PlayerStateAthena)
		return Athena_HandleStartingNewPlayerOriginal(GameMode, NewPlayerActor);

	static auto CharacterPartsOffset = PlayerStateAthena->GetOffset("CharacterParts", false);
	static auto CustomCharacterPartsStruct = FindObject<UStruct>(L"/Script/FortniteGame.CustomCharacterParts");
	auto CharacterParts = PlayerStateAthena->GetPtr<__int64>("CharacterParts");

	static auto PartsOffset = FindOffsetStruct("/Script/FortniteGame.CustomCharacterParts", "Parts", false);
	auto Parts = (UObject**)(__int64(CharacterParts) + PartsOffset); // UCustomCharacterPart* Parts[0x6]

	static auto CustomCharacterPartClass = FindObject<UClass>(L"/Script/FortniteGame.CustomCharacterPart");

	if (Globals::bNoMCP)
	{
		if (CharacterPartsOffset != -1) // && CustomCharacterPartsStruct)
		{
			static auto headPart = LoadObject("/Game/Characters/CharacterParts/Female/Medium/Heads/F_Med_Head1.F_Med_Head1", CustomCharacterPartClass);
			static auto bodyPart = LoadObject("/Game/Characters/CharacterParts/Female/Medium/Bodies/F_Med_Soldier_01.F_Med_Soldier_01", CustomCharacterPartClass);
			static auto backpackPart = LoadObject("/Game/Characters/CharacterParts/Backpacks/NoBackpack.NoBackpack", CustomCharacterPartClass);

			Parts[(int)EFortCustomPartType::Head] = headPart;
			Parts[(int)EFortCustomPartType::Body] = bodyPart;

			static auto OnRep_CharacterPartsFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerState.OnRep_CharacterParts");
			PlayerStateAthena->ProcessEvent(OnRep_CharacterPartsFn);
		}
	}

	static auto SquadIdOffset = PlayerStateAthena->GetOffset("SquadId", false);

	if (SquadIdOffset != -1)
		PlayerStateAthena->GetSquadId() = PlayerStateAthena->GetTeamIndex() - 3; // wrong place to do this

	TWeakObjectPtr<AFortPlayerStateAthena> WeakPlayerState{};
	WeakPlayerState.ObjectIndex = PlayerStateAthena->InternalIndex;
	WeakPlayerState.ObjectSerialNumber = GetItemByIndex(PlayerStateAthena->InternalIndex)->SerialNumber;

	if (auto TeamsArrayContainer = GameState->GetTeamsArrayContainer())
	{
		auto& SquadArray = TeamsArrayContainer->SquadsArray.at(PlayerStateAthena->GetSquadId());
		SquadArray.Add(WeakPlayerState);
	}

	LOG_INFO(LogDev, "New player going on TeamIndex {} with SquadId {}", PlayerStateAthena->GetTeamIndex(), SquadIdOffset != -1 ? PlayerStateAthena->GetSquadId() : -1);

	// idk if this is needed

	static auto bHasServerFinishedLoadingOffset = NewPlayer->GetOffset("bHasServerFinishedLoading");
	NewPlayer->Get<bool>(bHasServerFinishedLoadingOffset) = true;

	static auto OnRep_bHasServerFinishedLoadingFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.OnRep_bHasServerFinishedLoading");
	NewPlayer->ProcessEvent(OnRep_bHasServerFinishedLoadingFn);

	static auto bHasStartedPlayingOffset = PlayerStateAthena->GetOffset("bHasStartedPlaying");
	static auto bHasStartedPlayingFieldMask = GetFieldMask(PlayerStateAthena->GetProperty("bHasStartedPlaying"));
	PlayerStateAthena->SetBitfieldValue(bHasStartedPlayingOffset, bHasStartedPlayingFieldMask, true);

	static auto OnRep_bHasStartedPlayingFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerState.OnRep_bHasStartedPlaying");
	PlayerStateAthena->ProcessEvent(OnRep_bHasStartedPlayingFn);

	PlayerStateAthena->GetWorldPlayerId() = PlayerStateAthena->GetPlayerID();

	auto PlayerAbilitySet = GetPlayerAbilitySet();
	auto AbilitySystemComponent = PlayerStateAthena->GetAbilitySystemComponent();

	if (PlayerAbilitySet)
	{
		PlayerAbilitySet->GiveToAbilitySystem(AbilitySystemComponent);
	}

	struct FUniqueNetIdWrapper
	{
		unsigned char                                      UnknownData00[0x1];                                       // 0x0000(0x0001) MISSED OFFSET
	};

	struct FUniqueNetIdReplExperimental : public FUniqueNetIdWrapper
	{
		unsigned char                                      UnknownData00[0x17];                                      // 0x0001(0x0017) MISSED OFFSET
		TArray<unsigned char>                              ReplicationBytes;                                         // 0x0018(0x0010) (ZeroConstructor, Transient, Protected, NativeAccessSpecifierProtected)
	};

	static auto PlayerCameraManagerOffset = NewPlayer->GetOffset("PlayerCameraManager");
	auto PlayerCameraManager = NewPlayer->Get(PlayerCameraManagerOffset);

	if (PlayerCameraManager)
	{
		static auto ViewRollMinOffset = PlayerCameraManager->GetOffset("ViewRollMin");
		PlayerCameraManager->Get<float>(ViewRollMinOffset) = 0;

		static auto ViewRollMaxOffset = PlayerCameraManager->GetOffset("ViewRollMax");
		PlayerCameraManager->Get<float>(ViewRollMaxOffset) = 0;
	}

	static auto UniqueIdOffset = PlayerStateAthena->GetOffset("UniqueId");
	auto PlayerStateUniqueId = PlayerStateAthena->GetPtr<FUniqueNetIdRepl>(UniqueIdOffset);

	{
		static auto GameMemberInfoArrayOffset = GameState->GetOffset("GameMemberInfoArray", false);

		// if (false)
		if (GameMemberInfoArrayOffset != -1)
		// if (Engine_Version >= 423)
		{
			struct FGameMemberInfo : public FFastArraySerializerItem
			{
				unsigned char                                      SquadId;                                                  // 0x000C(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
				unsigned char                                      TeamIndex;                                                // 0x000D(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
				unsigned char                                      UnknownData00[0x2];                                       // 0x000E(0x0002) MISSED OFFSET
				FUniqueNetIdReplExperimental                            MemberUniqueId;                                           // 0x0010(0x0028) (HasGetValueTypeHash, NativeAccessSpecifierPublic)
			};

			static auto GameMemberInfoStructSize = 0x38;
			// LOG_INFO(LogDev, "Compare: 0x{:x} 0x{:x}", GameMemberInfoStructSize, sizeof(FGameMemberInfo));

			auto GameMemberInfo = Alloc<__int64>(GameMemberInfoStructSize);

			((FFastArraySerializerItem*)GameMemberInfo)->MostRecentArrayReplicationKey = -1;
			((FFastArraySerializerItem*)GameMemberInfo)->ReplicationID = -1;
			((FFastArraySerializerItem*)GameMemberInfo)->ReplicationKey = -1;

			if (false)
			{
				static auto GameMemberInfo_SquadIdOffset = 0x000C;
				static auto GameMemberInfo_TeamIndexOffset = 0x000D;
				static auto GameMemberInfo_MemberUniqueIdOffset = 0x0010;
				static auto UniqueIdSize = FUniqueNetIdRepl::GetSizeOfStruct();

				*(uint8*)(__int64(GameMemberInfo) + GameMemberInfo_SquadIdOffset) = PlayerStateAthena->GetSquadId();
				*(uint8*)(__int64(GameMemberInfo) + GameMemberInfo_TeamIndexOffset) = PlayerStateAthena->GetTeamIndex();
				CopyStruct((void*)(__int64(GameMemberInfo) + GameMemberInfo_MemberUniqueIdOffset), PlayerStateUniqueId, UniqueIdSize);
			}
			else
			{
				((FGameMemberInfo*)GameMemberInfo)->SquadId = PlayerStateAthena->GetSquadId();
				((FGameMemberInfo*)GameMemberInfo)->TeamIndex = PlayerStateAthena->GetTeamIndex();
				// GameMemberInfo->MemberUniqueId = PlayerStateUniqueId;
				((FUniqueNetIdRepl*)&((FGameMemberInfo*)GameMemberInfo)->MemberUniqueId)->CopyFromAnotherUniqueId(PlayerStateUniqueId);
			}

			static auto GameMemberInfoArray_MembersOffset = FindOffsetStruct("/Script/FortniteGame.GameMemberInfoArray", "Members");

			auto GameMemberInfoArray = GameState->GetPtr<FFastArraySerializer>(GameMemberInfoArrayOffset);

			((TArray<FGameMemberInfo>*)(__int64(GameMemberInfoArray) + GameMemberInfoArray_MembersOffset))->AddPtr(
				(FGameMemberInfo*)GameMemberInfo, GameMemberInfoStructSize
			);

			GameMemberInfoArray->MarkArrayDirty();
		}
	}

	if (Globals::bCreative)
	{
		static auto CreativePortalManagerOffset = GameState->GetOffset("CreativePortalManager");
		auto CreativePortalManager = GameState->Get(CreativePortalManagerOffset);

		static auto AvailablePortalsOffset = CreativePortalManager->GetOffset("AvailablePortals", false);

		AFortAthenaCreativePortal* Portal = nullptr;

		if (AvailablePortalsOffset != -1)
		{
			auto& AvailablePortals = CreativePortalManager->Get<TArray<AActor*>>(AvailablePortalsOffset);

			if (AvailablePortals.Num() > 0)
			{
				Portal = (AFortAthenaCreativePortal*)AvailablePortals.at(0);
				AvailablePortals.Remove(0);

				static auto UsedPortalsOffset = CreativePortalManager->GetOffset("UsedPortals");
				auto& UsedPortals = CreativePortalManager->Get<TArray<AActor*>>(UsedPortalsOffset);
				UsedPortals.Add(Portal);
			}
			else
			{
				LOG_WARN(LogCreative, "AvaliablePortals size is 0!");
			}
		}
		else
		{
			static auto AllPortalsOffset = CreativePortalManager->GetOffset("AllPortals");
			auto& AllPortals = CreativePortalManager->Get<TArray<AFortAthenaCreativePortal*>>(AllPortalsOffset);
		
			for (int i = 0; i < AllPortals.size(); i++)
			{
				auto CurrentPortal = AllPortals.at(i);

				if (!CurrentPortal->GetLinkedVolume() || CurrentPortal->GetLinkedVolume()->GetVolumeState() == EVolumeState::Ready)
					continue;

				Portal = CurrentPortal;
				break;
			}
		}

		if (Portal)
		{
			// Portal->GetCreatorName() = PlayerStateAthena->GetPlayerName();

			auto OwningPlayer = Portal->GetOwningPlayer();

			if (OwningPlayer != nullptr)
			{
				CopyStruct(OwningPlayer, PlayerStateUniqueId, FUniqueNetIdRepl::GetSizeOfStruct());
				// *(FUniqueNetIdReplExperimental*)OwningPlayer = PlayerStateUniqueId;
			}

			Portal->GetPortalOpen() = true;

			static auto PlayersReadyOffset = Portal->GetOffset("PlayersReady", false);

			if (PlayersReadyOffset != -1)
			{
				auto& PlayersReady = Portal->Get<TArray<FUniqueNetIdRepl>>(PlayersReadyOffset);
				PlayersReady.AddPtr(PlayerStateUniqueId, FUniqueNetIdRepl::GetSizeOfStruct()); // im not even sure what this is for
			}

			static auto bUserInitiatedLoadOffset = Portal->GetOffset("bUserInitiatedLoad", false);

			if (bUserInitiatedLoadOffset != -1)
				Portal->GetUserInitiatedLoad() = true;

			Portal->GetInErrorState() = false;

			static auto OwnedPortalOffset = NewPlayer->GetOffset("OwnedPortal");
			NewPlayer->Get<AFortAthenaCreativePortal*>(OwnedPortalOffset) = Portal;

			NewPlayer->GetCreativePlotLinkedVolume() = Portal->GetLinkedVolume();

			// OnRep_CreativePlotLinkedVolume ?

			Portal->GetLinkedVolume()->GetVolumeState() = EVolumeState::Ready;

			static auto IslandPlayset = FindObject<UFortPlaysetItemDefinition>("/Game/Playsets/PID_Playset_60x60_Composed.PID_Playset_60x60_Composed");

			if (auto Volume = NewPlayer->GetCreativePlotLinkedVolume())
			{
				// if (IslandPlayset)
					// Volume->UpdateSize({ (float)IslandPlayset->Get<int>("SizeX"), (float)IslandPlayset->Get<int>("SizeY"), (float)IslandPlayset->Get<int>("SizeZ") });

				static auto FortLevelSaveComponentClass = FindObject<UClass>("/Script/FortniteGame.FortLevelSaveComponent");
				auto LevelSaveComponent = (UObject*)Volume->GetComponentByClass(FortLevelSaveComponentClass);

				if (LevelSaveComponent)
				{
					static auto AccountIdOfOwnerOffset = LevelSaveComponent->GetOffset("AccountIdOfOwner");
					LevelSaveComponent->GetPtr<FUniqueNetIdRepl>(AccountIdOfOwnerOffset)->CopyFromAnotherUniqueId(PlayerStateUniqueId);
					// CopyStruct(LevelSaveComponent->GetPtr<FUniqueNetIdReplExperimental>(AccountIdOfOwnerOffset), PlayerStateUniqueId, FUniqueNetIdRepl::GetSizeOfStruct());
					// LevelSaveComponent->Get<FUniqueNetIdReplExperimental>(AccountIdOfOwnerOffset) = PlayerStateUniqueId;

					static auto bIsLoadedOffset = LevelSaveComponent->GetOffset("bIsLoaded");
					LevelSaveComponent->Get<bool>(bIsLoadedOffset) = true;
				}
			}

			UFortPlaysetItemDefinition::ShowPlayset(IslandPlayset, Portal->GetLinkedVolume());

			LOG_INFO(LogCreative, "Initialized player portal!");
		}
		else
		{
			LOG_INFO(LogCreative, "Failed to find an open portal!");
		}
	}

	LOG_INFO(LogDev, "HandleStartingNewPlayer end");

	if (Engine_Version <= 420)
	{
		static auto OverriddenBackpackSizeOffset = NewPlayer->GetOffset("OverriddenBackpackSize");
		LOG_INFO(LogDev, "NewPlayer->Get<int>(OverriddenBackpackSizeOffset): {}", NewPlayer->Get<int>(OverriddenBackpackSizeOffset));
		NewPlayer->Get<int>(OverriddenBackpackSizeOffset) = 5;
	}

	return Athena_HandleStartingNewPlayerOriginal(GameMode, NewPlayerActor);
}

void AFortGameModeAthena::SetZoneToIndexHook(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK)
{
	LOG_INFO(LogDev, "OverridePhaseMaybeIDFK: {}", OverridePhaseMaybeIDFK);
	return SetZoneToIndexOriginal(GameModeAthena, OverridePhaseMaybeIDFK);
}
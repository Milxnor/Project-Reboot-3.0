#include "FortGameModeAthena.h"

#include "FortPlayerControllerAthena.h"
#include "FortPlaysetItemDefinition.h"
#include "FortAthenaCreativePortal.h"
#include "BuildingContainer.h"
#include "MegaStormManager.h"
#include "FortLootPackage.h"
#include "FortPlayerPawn.h"
#include "FortPickup.h"

#include "NetSerialization.h"
#include "GameplayStatics.h"
#include "KismetStringLibrary.h"
#include "SoftObjectPtr.h"

#include "vehicles.h"
#include "globals.h"
#include "events.h"
#include "reboot.h"
#include "ai.h"
#include "Map.h"

enum class EDynamicFoundationEnabledState : uint8_t
{
	Unknown = 0,
	Enabled = 1,
	Disabled = 2,
	EDynamicFoundationEnabledState_MAX = 3
};

enum class EDynamicFoundationType : uint8_t
{
	Static = 0,
	StartEnabled_Stationary = 1,
	StartEnabled_Dynamic = 2,
	StartDisabled = 3,
	EDynamicFoundationType_MAX = 4
};

void ShowFoundation(UObject* BuildingFoundation)
{
	if (!BuildingFoundation)
		return;

	static auto bServerStreamedInLevelFieldMask = GetFieldMask(BuildingFoundation->GetProperty("bServerStreamedInLevel"));
	static auto bServerStreamedInLevelOffset = BuildingFoundation->GetOffset("bServerStreamedInLevel");
	BuildingFoundation->SetBitfieldValue(bServerStreamedInLevelOffset, bServerStreamedInLevelFieldMask, true);

	static auto DynamicFoundationTypeOffset = BuildingFoundation->GetOffset("DynamicFoundationType");
	BuildingFoundation->Get<uint8_t>(DynamicFoundationTypeOffset) = true ? 0 : 3;

	static auto bShowHLODWhenDisabledOffset = BuildingFoundation->GetOffset("bShowHLODWhenDisabled", false);

	if (bShowHLODWhenDisabledOffset != 0)
	{
		static auto bShowHLODWhenDisabledFieldMask = GetFieldMask(BuildingFoundation->GetProperty("bShowHLODWhenDisabled"));
		BuildingFoundation->SetBitfieldValue(bShowHLODWhenDisabledOffset, bShowHLODWhenDisabledFieldMask, true);
	}

	static auto OnRep_ServerStreamedInLevelFn = FindObject<UFunction>("/Script/FortniteGame.BuildingFoundation.OnRep_ServerStreamedInLevel");
	BuildingFoundation->ProcessEvent(OnRep_ServerStreamedInLevelFn);

	static auto DynamicFoundationRepDataOffset = BuildingFoundation->GetOffset("DynamicFoundationRepData", false);

	if (DynamicFoundationRepDataOffset != 0)
	{
		auto DynamicFoundationRepData = BuildingFoundation->GetPtr<void>(DynamicFoundationRepDataOffset);

		static auto EnabledStateOffset = FindOffsetStruct("/Script/FortniteGame.DynamicBuildingFoundationRepData", "EnabledState");

		*(EDynamicFoundationEnabledState*)(__int64(DynamicFoundationRepData) + EnabledStateOffset) = EDynamicFoundationEnabledState::Enabled;

		static auto OnRep_DynamicFoundationRepDataFn = FindObject<UFunction>("/Script/FortniteGame.BuildingFoundation.OnRep_DynamicFoundationRepData");
		BuildingFoundation->ProcessEvent(OnRep_DynamicFoundationRepDataFn);
	}

	static auto FoundationEnabledStateOffset = BuildingFoundation->GetOffset("FoundationEnabledState", false);

	if (FoundationEnabledStateOffset != 0)
		BuildingFoundation->Get<EDynamicFoundationEnabledState>(FoundationEnabledStateOffset) = EDynamicFoundationEnabledState::Enabled;
}

static void StreamLevel(std::string LevelName, FVector Location = {})
{
	static auto BuildingFoundation3x3Class = FindObject<UClass>("/Script/FortniteGame.BuildingFoundation3x3");
	FTransform Transform{};
	Transform.Scale3D = { 1, 1, 1 };
	Transform.Translation = Location;
	auto BuildingFoundation = (GetWorld()->SpawnActor<ABuildingSMActor>(BuildingFoundation3x3Class, Transform));

	if (!BuildingFoundation)
	{
		LOG_ERROR(LogGame, "Failed to spawn BuildingFoundation for streaming!");
		return;
	}

	// BuildingFoundation->InitializeBuildingActor(BuildingFoundation, nullptr, false);

	static auto FoundationNameOffset = FindOffsetStruct("/Script/FortniteGame.BuildingFoundationStreamingData", "FoundationName");
	static auto FoundationLocationOffset = FindOffsetStruct("/Script/FortniteGame.BuildingFoundationStreamingData", "FoundationLocation");
	static auto StreamingDataOffset = BuildingFoundation->GetOffset("StreamingData");
	static auto LevelToStreamOffset = BuildingFoundation->GetOffset("LevelToStream");

	auto StreamingData = BuildingFoundation->GetPtr<__int64>(StreamingDataOffset);

	*(FName*)(__int64(StreamingData) + FoundationNameOffset) = UKismetStringLibrary::Conv_StringToName(std::wstring(LevelName.begin(), LevelName.end()).c_str());
	*(FVector*)(__int64(StreamingData) + FoundationLocationOffset) = Location;

	*(FName*)(__int64(BuildingFoundation) + LevelToStreamOffset) = UKismetStringLibrary::Conv_StringToName(std::wstring(LevelName.begin(), LevelName.end()).c_str());
	
	static auto OnRep_LevelToStreamFn = FindObject<UFunction>("/Script/FortniteGame.BuildingFoundation.OnRep_LevelToStream");
	BuildingFoundation->ProcessEvent(OnRep_LevelToStreamFn);

	ShowFoundation(BuildingFoundation);
}

UObject* GetPlaylistToUse()
{
	auto Playlist = FindObject("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");

	if (Globals::bCreative)
		Playlist = FindObject("/Game/Athena/Playlists/Creative/Playlist_PlaygroundV2.Playlist_PlaygroundV2");

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

	// Playlist = FindObject("/MoleGame/Playlists/Playlist_MoleGame.Playlist_MoleGame");
	// Playlist = FindObject("/Game/Athena/Playlists/DADBRO/Playlist_DADBRO_Squads_8.Playlist_DADBRO_Squads_8");

	return Playlist;
}

FName AFortGameModeAthena::RedirectLootTier(const FName& LootTier)
{
	static auto RedirectAthenaLootTierGroupsOffset = this->GetOffset("RedirectAthenaLootTierGroups", false);

	if (RedirectAthenaLootTierGroupsOffset == 0)
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

	for (int i = 0; i < RedirectAthenaLootTierGroups.Pairs.Elements.Num(); i++)
	{
		auto& Pair = RedirectAthenaLootTierGroups.Pairs.Elements.Data.at(i).ElementData.Value;

		auto& Key = Pair.Key();
		auto& Value = Pair.Value();

		// LOG_INFO(LogDev, "[{}] {} {}", i, Key.ComparisonIndex.Value ? Key.ToString() : "NULL", Key.ComparisonIndex.Value ? Value.ToString() : "NULL");

		if (Key == LootTier)
			return Value;
	}

	return LootTier;
}

bool AFortGameModeAthena::Athena_ReadyToStartMatchHook(AFortGameModeAthena* GameMode)
{
	auto GameState = GameMode->GetGameStateAthena();

	auto SetPlaylist = [&GameState](UObject* Playlist) -> void {
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
				if (Fortnite_Version >= 13)
				{
					static auto LastSafeZoneIndexOffset = aeuh->GetOffset("LastSafeZoneIndex");

					if (LastSafeZoneIndexOffset != -1)
					{
						*(int*)(__int64(aeuh) + LastSafeZoneIndexOffset) = 0;
					}
				}
			}

			GameState->OnRep_CurrentPlaylistInfo();
		}
		else
		{
			GameState->Get("CurrentPlaylistData") = Playlist;
			GameState->OnRep_CurrentPlaylistInfo(); // calls OnRep_CurrentPlaylistData
		}
	};

	auto& LocalPlayers = GetLocalPlayers();

	if (LocalPlayers.Num() && LocalPlayers.Data)
	{
		LocalPlayers.Remove(0);
	}

	static int LastNum2 = 1;

	if (AmountOfRestarts != LastNum2)
	{
		LastNum2 = AmountOfRestarts;

		LOG_INFO(LogDev, "Presetup!");

		GameMode->Get<int>("WarmupRequiredPlayerCount") = 1;	
		
		{
			SetPlaylist(GetPlaylistToUse());
			LOG_INFO(LogDev, "Set playlist!");
		}
		
		// if (false)
		{
			auto Fortnite_Season = std::floor(Fortnite_Version);

			if (Fortnite_Season == 6)
			{
				if (Fortnite_Version != 6.10)
				{
					auto Lake = FindObject(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Lake1"));
					auto Lake2 = FindObject("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Lake2");

					Fortnite_Version <= 6.21 ? ShowFoundation(Lake) : ShowFoundation(Lake2);
					// ^ This shows the lake after or before the event i dont know if this is needed.
				}
				else
				{
					auto Lake = FindObject(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_StreamingTest12"));
					ShowFoundation(Lake);
				}

				auto FloatingIsland = Fortnite_Version == 6.10 ? FindObject(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_StreamingTest13")) :
					FindObject(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_FloatingIsland"));

				ShowFoundation(FloatingIsland);

				UObject* Scripting = FindObject("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.BP_IslandScripting3"); // bruh

				if (Scripting)
				{
					static auto UpdateMapOffset = Scripting->GetOffset("UpdateMap", false);

					if (UpdateMapOffset != 0)
					{
						Scripting->Get<bool>(UpdateMapOffset) = true;

						static auto OnRep_UpdateMap = FindObject<UFunction>("/Game/Athena/Prototype/Blueprints/Island/BP_IslandScripting.BP_IslandScripting_C.OnRep_UpdateMap");
						Scripting->ProcessEvent(OnRep_UpdateMap);
					}
				}
			}

			if (Fortnite_Season >= 7 && Fortnite_Season <= 10)
			{
				if (Fortnite_Season == 7)
				{
					if (Fortnite_Version == 7.30)
					{
						auto PleasantParkIdk = FindObject(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.PleasentParkFestivus"));
						ShowFoundation(PleasantParkIdk);

						auto PleasantParkGround = FindObject("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.PleasentParkDefault");
						ShowFoundation(PleasantParkGround);
					}

					auto PolarPeak = FindObject(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_POI_25x36"));
					ShowFoundation(PolarPeak);

					auto tiltedtower = FindObject("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.ShopsNew");
					ShowFoundation(tiltedtower); // 7.40 specific?
				}

				else if (Fortnite_Season == 8)
				{
					auto Volcano = FindObject(("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_POI_50x53_Volcano"));
					ShowFoundation(Volcano);
				}

				else if (Fortnite_Season == 10)
				{
					if (Fortnite_Version >= 10.20)
					{
						auto Island = FindObject("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_StreamingTest16");
						ShowFoundation(Island);
					}
				}

				auto TheBlock = FindObject("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.SLAB_2"); // SLAB_3 is blank
				ShowFoundation(TheBlock);
			}
	
			if (Fortnite_Version == 14.60 && Globals::bGoingToPlayEvent)
			{
				auto aircraftcarrier = FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.Lobby_Foundation3");
				LOG_INFO(LogDev, "aircraftcarrier: {}", __int64(aircraftcarrier));
				ShowFoundation(aircraftcarrier);
			}

			if (Fortnite_Version == 17.50) {
				auto FarmAfter = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.farmbase_2"));
				ShowFoundation(FarmAfter);

				auto FarmPhase = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.Farm_Phase_03")); // Farm Phases (Farm_Phase_01, Farm_Phase_02 and Farm_Phase_03)
				ShowFoundation(FarmPhase);
			}

			if (Fortnite_Version == 17.40) {
				auto AbductedCoral = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.CoralPhase_02")); // Coral Castle Phases (CoralPhase_01, CoralPhase_02 and CoralPhase_03)
				ShowFoundation(AbductedCoral);

				auto CoralFoundation_01 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation_0"));
				ShowFoundation(CoralFoundation_01);

				auto CoralFoundation_05 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation6"));
				ShowFoundation(CoralFoundation_05);

				auto CoralFoundation_07 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation3"));
				ShowFoundation(CoralFoundation_07);

				auto CoralFoundation_10 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation2_1"));
				ShowFoundation(CoralFoundation_10);

				auto CoralFoundation_13 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation4"));
				ShowFoundation(CoralFoundation_13);

				auto CoralFoundation_17 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation5"));
				ShowFoundation(CoralFoundation_17);
			}

			if (Fortnite_Version == 17.30) {
				auto AbductedSlurpy = FindObject(("LF_Athena_POI_50x50_C /Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.Slurpy_Phase03")); // Slurpy Swamp Phases (Slurpy_Phase01, Slurpy_Phase02 and Slurpy_Phase03)
				ShowFoundation(AbductedSlurpy);
			}

			if (Fortnite_Season == 13)
			{
				auto SpawnIsland = FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.Lobby_Foundation");
				ShowFoundation(SpawnIsland);

				// SpawnIsland->RepData->Soemthing = FoundationSetup->LobbyLocation;
			}

			if (Fortnite_Version == 12.41)
			{
				auto JS03 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_Athena_POI_19x19_2"));
				ShowFoundation(JS03);

				auto JH00 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head6_18"));
				ShowFoundation(JH00);

				auto JH01 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head5_14"));
				ShowFoundation(JH01);

				auto JH02 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head3_8"));
				ShowFoundation(JH02);

				auto JH03 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head_2"));
				ShowFoundation(JH03);

				auto JH04 = FindObject(("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head4_11"));
				ShowFoundation(JH04);
			}

			auto PlaylistToUse = GetPlaylistToUse();

			if (PlaylistToUse)
			{
				static auto AdditionalLevelsOffset = PlaylistToUse->GetOffset("AdditionalLevels", false);

				if (AdditionalLevelsOffset != 0)
				{
					auto& AdditionalLevels = PlaylistToUse->Get<TArray<TSoftObjectPtr<UClass>>>(AdditionalLevelsOffset);

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

		SetBitfield(GameMode->GetPtr<PlaceholderBitfield>("bWorldIsReady"), 1, true);
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

	if (AmountOfRestarts != LastNum5 && LastNum6 == AmountOfRestarts)
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

	static auto FortPlayerStartWarmupClass = FindObject<UClass>("/Script/FortniteGame.FortPlayerStartWarmup");
	TArray<AActor*> Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortPlayerStartWarmupClass);

	int ActorsNum = Actors.Num();

	Actors.Free();

	if (ActorsNum == 0)
		return false;

	static int LastNum9 = 1;

	if (AmountOfRestarts != LastNum9)
	{
		LastNum9 = AmountOfRestarts;
	}

	static auto MapInfoOffset = GameState->GetOffset("MapInfo");
	auto MapInfo = GameState->Get(MapInfoOffset);
	
	if (!MapInfo && Engine_Version >= 421)
		return false;

	// if (GameState->GetPlayersLeft() < GameMode->Get<int>("WarmupRequiredPlayerCount"))
	// if (!bFirstPlayerJoined)
		// return false;

	static int LastNum = 1;

	if (AmountOfRestarts != LastNum)
	{
		LastNum = AmountOfRestarts;

		float Duration = 1000.f;
		float EarlyDuration = Duration;

		float TimeSeconds = 35.f; // UGameplayStatics::GetTimeSeconds(GetWorld());

		LOG_INFO(LogDev, "Initializing!");
		LOG_INFO(LogDev, "GameMode 0x{:x}", __int64(GameMode));

		GameState->Get<float>("WarmupCountdownEndTime") = TimeSeconds + Duration;
		GameMode->Get<float>("WarmupCountdownDuration") = Duration;

		GameState->Get<float>("WarmupCountdownStartTime") = TimeSeconds;
		GameMode->Get<float>("WarmupEarlyCountdownDuration") = EarlyDuration;

		/*
		auto AllMegaStormManagers = UGameplayStatics::GetAllActorsOfClass(GetWorld(), GameMode->Get<UClass*>("MegaStormManagerClass"));

		LOG_INFO(LogDev, "AllMegaStormManagers.Num() {}", AllMegaStormManagers.Num());

		if (AllMegaStormManagers.Num())
		{
			auto MegaStormManager = (AMegaStormManager*)AllMegaStormManagers.at(0); // GameMode->Get<AMegaStormManager*>(MegaStormManagerOffset);

			LOG_INFO(LogDev, "MegaStormManager {}", __int64(MegaStormManager));

			if (MegaStormManager)
			{
				LOG_INFO(LogDev, "MegaStormManager->GetMegaStormCircles().Num() {}", MegaStormManager->GetMegaStormCircles().Num());
			}
		}
		*/

		// GameState->Get<bool>("bGameModeWillSkipAircraft") = Globals::bGoingToPlayEvent && Fortnite_Version == 17.30;

		// if (Engine_Version < 424)
			GameState->OnRep_CurrentPlaylistInfo(); // ?

		// SetupNavConfig();

		LOG_INFO(LogDev, "Initialized!");
	}

	if (Engine_Version >= 424) // returning true is stripped on c2+
	{
		if (GameState->GetPlayersLeft() >= GameMode->Get<int>("WarmupRequiredPlayerCount"))
		{
			if (MapInfo)
			{
				static auto FlightInfosOffset = MapInfo->GetOffset("FlightInfos");

				if (MapInfo->Get<TArray<__int64>>(FlightInfosOffset).ArrayNum <= 0)
					return true;
			}
		}
	}

	static auto TeamsOffset = GameState->GetOffset("Teams");
	auto& Teams = GameState->Get<TArray<UObject*>>(TeamsOffset);

	if (Teams.Num() <= 0)
		return false;

	static int LastNum3 = 1;

	if (AmountOfRestarts != LastNum3)
	{
		LastNum3 = AmountOfRestarts;

		LOG_INFO(LogNet, "Attempting to listen!");

		GetWorld()->Listen();

		// GameState->OnRep_CurrentPlaylistInfo();

		// return false;
	}

	return Athena_ReadyToStartMatchOriginal(GameMode);
}

int AFortGameModeAthena::Athena_PickTeamHook(AFortGameModeAthena* GameMode, uint8 preferredTeam, AActor* Controller)
{
	LOG_INFO(LogTeam, "PickTeam called!");
	static auto NextTeamIndex = 3;
	return NextTeamIndex;
}

void AFortGameModeAthena::Athena_HandleStartingNewPlayerHook(AFortGameModeAthena* GameMode, AActor* NewPlayerActor)
{
	if (!NewPlayerActor)
		return;

	LOG_INFO(LogPlayer, "HandleStartingNewPlayer!");

	static bool bFirst = Engine_Version >= 424;

	auto GameState = GameMode->GetGameStateAthena();

	if (bFirst)
	{
		bFirst = false;

		GameState->GetGamePhase() = EAthenaGamePhase::Warmup;
		GameState->OnRep_GamePhase();
	}

	static bool bSpawnedFloorLoot = false;

	if (!bSpawnedFloorLoot)
	{
		bSpawnedFloorLoot = true;

		auto SpawnIsland_FloorLoot = FindObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");
		auto BRIsland_FloorLoot = FindObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");

		TArray<AActor*> SpawnIsland_FloorLoot_Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnIsland_FloorLoot);

		TArray<AActor*> BRIsland_FloorLoot_Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), BRIsland_FloorLoot);

		auto SpawnIslandTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaFloorLoot_Warmup");
		auto BRIslandTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaFloorLoot");

		float UpZ = 50;

		EFortPickupSourceTypeFlag SpawnFlag = EFortPickupSourceTypeFlag::Container;

		bool bPrintWarmup = false;

		for (int i = 0; i < SpawnIsland_FloorLoot_Actors.Num(); i++)
		{
			ABuildingContainer* CurrentActor = (ABuildingContainer*)SpawnIsland_FloorLoot_Actors.at(i);

			// CurrentActor->K2_DestroyActor();
			// continue;

			auto Location = CurrentActor->GetActorLocation();
			Location.Z += UpZ;

			std::vector<LootDrop> LootDrops = PickLootDrops(SpawnIslandTierGroup, bPrintWarmup);

			if (bPrintWarmup)
			{
				std::cout << "\n\n";
			}

			if (LootDrops.size())
			{
				for (auto& LootDrop : LootDrops)
					AFortPickup::SpawnPickup(LootDrop.ItemDefinition, Location, LootDrop.Count, SpawnFlag, EFortPickupSpawnSource::Unset, LootDrop.LoadedAmmo);
			}
		}

		bool bPrint = false;

		int spawned = 0;

		for (int i = 0; i < BRIsland_FloorLoot_Actors.Num(); i++)
		{
			ABuildingContainer* CurrentActor = (ABuildingContainer*)BRIsland_FloorLoot_Actors.at(i);

			// CurrentActor->K2_DestroyActor();
			spawned++;
			// continue;

			auto Location = CurrentActor->GetActorLocation();
			Location.Z += UpZ;

			std::vector<LootDrop> LootDrops = PickLootDrops(BRIslandTierGroup, bPrint);

			if (bPrint)
				std::cout << "\n";

			if (LootDrops.size())
			{
				for (auto& LootDrop : LootDrops)
					AFortPickup::SpawnPickup(LootDrop.ItemDefinition, Location, LootDrop.Count, SpawnFlag, EFortPickupSpawnSource::Unset, LootDrop.LoadedAmmo);
			}
		}
	}

	static bool bSpawnedVehicles = Engine_Version < 423;

	if (!bSpawnedVehicles)
	{
		bSpawnedVehicles = true;

		SpawnVehicles();
	}

	auto NewPlayer = (AFortPlayerControllerAthena*)NewPlayerActor;

	auto PlayerStateAthena = NewPlayer->GetPlayerStateAthena();

	if (Globals::bNoMCP)
	{
		static auto CharacterPartsOffset = PlayerStateAthena->GetOffset("CharacterParts", false);
		static auto CustomCharacterPartsStruct = FindObject<UStruct>("/Script/FortniteGame.CustomCharacterParts");

		if (CharacterPartsOffset != 0) // && CustomCharacterPartsStruct)
		{
			auto CharacterParts = PlayerStateAthena->GetPtr<__int64>("CharacterParts");

			static auto PartsOffset = FindOffsetStruct("/Script/FortniteGame.CustomCharacterParts", "Parts");
			auto Parts = (UObject**)(__int64(CharacterParts) + PartsOffset); // UCustomCharacterPart* Parts[0x6]

			static auto headPart = LoadObject("/Game/Characters/CharacterParts/Female/Medium/Heads/F_Med_Head1.F_Med_Head1");
			static auto bodyPart = LoadObject("/Game/Characters/CharacterParts/Female/Medium/Bodies/F_Med_Soldier_01.F_Med_Soldier_01");

			Parts[(int)EFortCustomPartType::Head] = headPart;
			Parts[(int)EFortCustomPartType::Body] = bodyPart;

			static auto OnRep_CharacterPartsFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerState.OnRep_CharacterParts");
			PlayerStateAthena->ProcessEvent(OnRep_CharacterPartsFn);
		}
	}

	PlayerStateAthena->GetSquadId() = PlayerStateAthena->GetTeamIndex() - 2;

	// if (false)
	{
		// idk if this is needed

		static auto bHasServerFinishedLoadingOffset = NewPlayer->GetOffset("bHasServerFinishedLoading");
		NewPlayer->Get<bool>(bHasServerFinishedLoadingOffset) = true;

		static auto OnRep_bHasServerFinishedLoadingFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.OnRep_bHasServerFinishedLoading");
		NewPlayer->ProcessEvent(OnRep_bHasServerFinishedLoadingFn);

		static auto bHasStartedPlayingOffset = PlayerStateAthena->GetOffset("bHasStartedPlaying");
		PlayerStateAthena->Get<bool>(bHasStartedPlayingOffset) = true; // this is a bitfield!!!

		static auto OnRep_bHasStartedPlayingFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerState.OnRep_bHasStartedPlaying");
		PlayerStateAthena->ProcessEvent(OnRep_bHasStartedPlayingFn);
	}

	// static int CurrentPlayerId = 1;
	static auto PlayerIdOffset = PlayerStateAthena->GetOffset("PlayerId");
	PlayerStateAthena->GetWorldPlayerId() = PlayerStateAthena->Get<int>(PlayerIdOffset); // ++CurrentPlayerId;

	if (Globals::bAbilitiesEnabled)
	{
		static auto GameplayAbilitySet = Fortnite_Version >= 8.30 ? // LoadObject<UObject>(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") ? 
			LoadObject<UObject>(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") :
			LoadObject<UObject>(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer");

		LOG_INFO(LogDev, "GameplayAbilitySet {}", __int64(GameplayAbilitySet));

		if (GameplayAbilitySet)
		{
			LOG_INFO(LogDev, "GameplayAbilitySet Name {}", GameplayAbilitySet->GetName());
			static auto GameplayAbilitiesOffset = GameplayAbilitySet->GetOffset("GameplayAbilities");
			auto GameplayAbilities = GameplayAbilitySet->GetPtr<TArray<UClass*>>(GameplayAbilitiesOffset);

			for (int i = 0; i < GameplayAbilities->Num(); i++)
			{
				UClass* AbilityClass = GameplayAbilities->At(i);

				// LOG_INFO(LogDev, "AbilityClass {}", __int64(AbilityClass));

				if (!AbilityClass)
					continue;

				// LOG_INFO(LogDev, "AbilityClass Name {}", AbilityClass->GetFullName());

				// LOG_INFO(LogDev, "DefaultAbility {}", __int64(DefaultAbility));
				// LOG_INFO(LogDev, "DefaultAbility Name {}", DefaultAbility->GetFullName());

				PlayerStateAthena->GetAbilitySystemComponent()->GiveAbilityEasy(AbilityClass);
			}
		}
	}
	
	static auto GameMemberInfoArrayOffset = GameState->GetOffset("GameMemberInfoArray", false);

	struct FUniqueNetIdReplExperimental
	{
		unsigned char ahh[0x0028];
	};

	FUniqueNetIdReplExperimental bugha{};
	auto& PlayerStateUniqueId = PlayerStateAthena->Get<FUniqueNetIdReplExperimental>("UniqueId");

	// if (false)
	// if (GameMemberInfoArrayOffset != 0)
	if (Engine_Version >= 423)
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

		auto GameMemberInfo = Alloc<FGameMemberInfo>(GameMemberInfoStructSize);

		((FFastArraySerializerItem*)GameMemberInfo)->MostRecentArrayReplicationKey = -1;
		((FFastArraySerializerItem*)GameMemberInfo)->ReplicationID = -1;
		((FFastArraySerializerItem*)GameMemberInfo)->ReplicationKey = -1;

		if (false)
		{
			static auto GameMemberInfo_SquadIdOffset = 0x000C;
			static auto GameMemberInfo_TeamIndexOffset = 0x000D;
			static auto GameMemberInfo_MemberUniqueIdOffset = 0x0010;
			static auto UniqueIdSize = 0x0028;

			*(uint8*)(__int64(GameMemberInfo) + GameMemberInfo_SquadIdOffset) = PlayerStateAthena->GetSquadId();
			*(uint8*)(__int64(GameMemberInfo) + GameMemberInfo_TeamIndexOffset) = PlayerStateAthena->GetTeamIndex();
			CopyStruct((void*)(__int64(GameMemberInfo) + GameMemberInfo_MemberUniqueIdOffset), PlayerStateAthena->Get<void*>("UniqueId"), UniqueIdSize);

		}
		else
		{
			GameMemberInfo->SquadId = PlayerStateAthena->GetSquadId();
			GameMemberInfo->TeamIndex = PlayerStateAthena->GetTeamIndex();
			GameMemberInfo->MemberUniqueId = PlayerStateUniqueId;
		}

		static auto GameMemberInfoArray_MembersOffset = FindOffsetStruct("/Script/FortniteGame.GameMemberInfoArray", "Members");

		auto GameMemberInfoArray = GameState->GetPtr<FFastArraySerializer>(GameMemberInfoArrayOffset);

		((TArray<FGameMemberInfo>*)(__int64(GameMemberInfoArray) + GameMemberInfoArray_MembersOffset))->Add(*GameMemberInfo, GameMemberInfoStructSize);
		GameMemberInfoArray->MarkArrayDirty();
	}

	if (Globals::bCreative)
	{
		static auto CreativePortalManagerOffset = GameState->GetOffset("CreativePortalManager");
		auto CreativePortalManager = GameState->Get(CreativePortalManagerOffset);

		static auto AvailablePortalsOffset = CreativePortalManager->GetOffset("AvailablePortals", false);

		AFortAthenaCreativePortal* Portal = nullptr;

		if (AvailablePortalsOffset != 0)
		{
			auto& AvailablePortals = CreativePortalManager->Get<TArray<AActor*>>(AvailablePortalsOffset);
			Portal = (AFortAthenaCreativePortal*)AvailablePortals.at(0);
			AvailablePortals.Remove(0);

			static auto UsedPortalsOffset = CreativePortalManager->GetOffset("UsedPortals");
			auto& UsedPortals = CreativePortalManager->Get<TArray<AActor*>>(UsedPortalsOffset);
			UsedPortals.Add(Portal);
		}
		else
		{
			static auto AllPortalsOffset = CreativePortalManager->GetOffset("AllPortals");
			auto& AllPortals = CreativePortalManager->Get<TArray<AFortAthenaCreativePortal*>>(AllPortalsOffset);
		
			for (int i = 0; i < AllPortals.size(); i++)
			{
				auto CurrentPortal = AllPortals.at(i);

				if (CurrentPortal->GetUserInitiatedLoad())
					continue;

				Portal = CurrentPortal;
				break;
			}
		}

		if (Portal)
		{
			// Portal->GetCreatorName() = PlayerStateAthena->GetPlayerName();

			*(FUniqueNetIdReplExperimental*)Portal->GetOwningPlayer() = PlayerStateUniqueId;
			Portal->GetPortalOpen() = true;

			static auto PlayersReadyOffset = Portal->GetOffset("PlayersReady");
			auto& PlayersReady = Portal->Get<TArray<FUniqueNetIdReplExperimental>>(PlayersReadyOffset);
			PlayersReady.Add(PlayerStateUniqueId);

			Portal->GetUserInitiatedLoad() = true;
			Portal->GetInErrorState() = false;

			static auto OwnedPortalOffset = NewPlayer->GetOffset("OwnedPortal");
			NewPlayer->Get<AFortAthenaCreativePortal*>(OwnedPortalOffset) = Portal;

			static auto CreativePlotLinkedVolumeOffset = NewPlayer->GetOffset("CreativePlotLinkedVolume");
			NewPlayer->Get<AFortVolume*>(CreativePlotLinkedVolumeOffset) = Portal->GetLinkedVolume();

			Portal->GetLinkedVolume()->GetVolumeState() = EVolumeState::Ready;

			static auto IslandPlayset = FindObject<UFortPlaysetItemDefinition>("/Game/Playsets/PID_Playset_60x60_Composed.PID_Playset_60x60_Composed");

			UFortPlaysetItemDefinition::ShowPlayset(IslandPlayset, Portal->GetLinkedVolume());
		}
		else
		{
			LOG_INFO(LogCreative, "Failed to find an open portal!");
		}
	}

	static auto SpawnActorDataListOffset = GameMode->GetOffset("SpawnActorDataList", false);

	if (SpawnActorDataListOffset != 0)
	{
		auto& SpawnActorDataList = GameMode->Get<TArray<__int64>>(SpawnActorDataListOffset);
		LOG_INFO(LogDev, "SpawnActorDataList.Num(): {}", SpawnActorDataList.Num());
	}

	return Athena_HandleStartingNewPlayerOriginal(GameMode, NewPlayerActor);
}

void AFortGameModeAthena::SetZoneToIndexHook(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK)
{
	LOG_INFO(LogDev, "OverridePhaseMaybeIDFK: {}", OverridePhaseMaybeIDFK);
	return SetZoneToIndexOriginal(GameModeAthena, OverridePhaseMaybeIDFK);
}
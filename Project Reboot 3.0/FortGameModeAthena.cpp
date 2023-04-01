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
#include "OnlineReplStructs.h"

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


static UObject* GetPlaylistToUse()
{
	auto Playlist = FindObject("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
	// Playlist = FindObject("/BlueCheese/Playlists/Playlist_ShowdownAlt_BlueCheese_Trios.Playlist_ShowdownAlt_BlueCheese_Trios");

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

	// Playlist = FindObject("/Game/Athena/Playlists/Playground/Playlist_Playground.Playlist_Playground");

	// Playlist = FindObject("/Game/Athena/Playlists/Carmine/Playlist_Carmine.Playlist_Carmine");

	// Playlist = FindObject("/MoleGame/Playlists/Playlist_MoleGame.Playlist_MoleGame");
	// Playlist = FindObject("/Game/Athena/Playlists/DADBRO/Playlist_DADBRO_Squads_8.Playlist_DADBRO_Squads_8");

	// Playlist = FindObject("/Game/Athena/Playlists/Low/Playlist_Low_Solo.Playlist_Low_Solo");

	if (Globals::bCreative)
		Playlist = FindObject("/Game/Athena/Playlists/Creative/Playlist_PlaygroundV2.Playlist_PlaygroundV2");

	return Playlist;
}

void ShowFoundation(UObject* BuildingFoundation)
{
	if (!BuildingFoundation)
	{
		LOG_WARN(LogGame, "Attempting to show invalid building foundation.");
		return;
	}

	static auto bServerStreamedInLevelFieldMask = GetFieldMask(BuildingFoundation->GetProperty("bServerStreamedInLevel"));
	static auto bServerStreamedInLevelOffset = BuildingFoundation->GetOffset("bServerStreamedInLevel");
	BuildingFoundation->SetBitfieldValue(bServerStreamedInLevelOffset, bServerStreamedInLevelFieldMask, true);

	static auto DynamicFoundationTypeOffset = BuildingFoundation->GetOffset("DynamicFoundationType");
	BuildingFoundation->Get<uint8_t>(DynamicFoundationTypeOffset) = true ? 0 : 3;

	static auto bShowHLODWhenDisabledOffset = BuildingFoundation->GetOffset("bShowHLODWhenDisabled", false);

	if (bShowHLODWhenDisabledOffset != -1)
	{
		static auto bShowHLODWhenDisabledFieldMask = GetFieldMask(BuildingFoundation->GetProperty("bShowHLODWhenDisabled"));
		BuildingFoundation->SetBitfieldValue(bShowHLODWhenDisabledOffset, bShowHLODWhenDisabledFieldMask, true);
	}

	static auto OnRep_ServerStreamedInLevelFn = FindObject<UFunction>("/Script/FortniteGame.BuildingFoundation.OnRep_ServerStreamedInLevel");
	BuildingFoundation->ProcessEvent(OnRep_ServerStreamedInLevelFn);

	static auto DynamicFoundationRepDataOffset = BuildingFoundation->GetOffset("DynamicFoundationRepData", false);

	if (DynamicFoundationRepDataOffset != -1)
	{
		auto DynamicFoundationRepData = BuildingFoundation->GetPtr<void>(DynamicFoundationRepDataOffset);

		static auto EnabledStateOffset = FindOffsetStruct("/Script/FortniteGame.DynamicBuildingFoundationRepData", "EnabledState");

		*(EDynamicFoundationEnabledState*)(__int64(DynamicFoundationRepData) + EnabledStateOffset) = EDynamicFoundationEnabledState::Enabled;

		static auto OnRep_DynamicFoundationRepDataFn = FindObject<UFunction>("/Script/FortniteGame.BuildingFoundation.OnRep_DynamicFoundationRepData");
		BuildingFoundation->ProcessEvent(OnRep_DynamicFoundationRepDataFn);
	}

	static auto FoundationEnabledStateOffset = BuildingFoundation->GetOffset("FoundationEnabledState", false);

	if (FoundationEnabledStateOffset != -1)
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

UClass* AFortGameModeAthena::GetVehicleClassOverride(UClass* DefaultClass)
{
	static auto GetVehicleClassOverrideFn = FindObject<UFunction>("/Script/FortniteGame.FortGameModeAthena.GetVehicleClassOverride");

	if (!GetVehicleClassOverrideFn)
		return DefaultClass;

	struct { UClass* DefaultClass; UClass* ReturnValue; } GetVehicleClassOverride_Params{DefaultClass};
	
	this->ProcessEvent(GetVehicleClassOverrideFn, &GetVehicleClassOverride_Params);

	return GetVehicleClassOverride_Params.ReturnValue;
}

bool AFortGameModeAthena::Athena_ReadyToStartMatchHook(AFortGameModeAthena* GameMode)
{
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
			GameState->Get("CurrentPlaylistData") = Playlist;
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

	// LOG_INFO(LogDev, "ReadyToStartMatch AmountOfRestarts: {} LastNum2: {}!", AmountOfRestarts, LastNum2);

	if (AmountOfRestarts != LastNum2)
	{
		LastNum2 = AmountOfRestarts;

		LOG_INFO(LogDev, "Presetup!");

		GameMode->Get<int>("WarmupRequiredPlayerCount") = 1;
		
		auto PlaylistToUse = GetPlaylistToUse();

		if (!PlaylistToUse)
		{
			LOG_ERROR(LogPlaylist, "Failed to find playlist! Proceeding, but will probably not work as expected!");
		}
		else
		{
			if (Fortnite_Version >= 4)
				SetPlaylist(PlaylistToUse, true);

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

					if (UpdateMapOffset != -1)
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

					ShowFoundation(FindObject("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_POI_25x36")); // Polar Peak
					ShowFoundation(FindObject("/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.ShopsNew")); // Tilted Tower Shops, is this 7.40 specific?
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
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.Lobby_Foundation3")); // Aircraft Carrier
			}

			if (Fortnite_Version == 17.50) {
				auto FarmAfter = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.farmbase_2"));
				ShowFoundation(FarmAfter);

				auto FarmPhase = FindObject(("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.Farm_Phase_03")); // Farm Phases (Farm_Phase_01, Farm_Phase_02 and Farm_Phase_03)
				ShowFoundation(FarmPhase);
			}

			if (Fortnite_Version == 17.40) {
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.CoralPhase_02")); // Coral Castle Phases (CoralPhase_01, CoralPhase_02 and CoralPhase_03)
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation_0")); // CoralFoundation_01
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation6")); // CoralFoundation_05
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation3")); // CoralFoundation_07
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation2_1")); // CoralFoundation_10
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation4"));
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.LF_Athena_16x16_Foundation5"));
			}

			if (Fortnite_Version == 17.30) {
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_Mother.Apollo_Mother.PersistentLevel.Slurpy_Phase03")); // There are 1, 2 and 3
			}

			if (Fortnite_Season == 13)
			{
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.Lobby_Foundation"));

				// SpawnIsland->RepData->Soemthing = FoundationSetup->LobbyLocation;
			}

			if (Fortnite_Version == 12.41)
			{
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_Athena_POI_19x19_2"));
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head6_18"));
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head5_14"));
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head3_8"));
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head_2"));
				ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_Jerky_Head4_11"));
			}

			if (Fortnite_Version == 11.31)
			{
				// There are also christmas trees & stuff but we need to find a better way to stream that because there's a lot.

				if (false) // If the client loads this, it says the package doesnt exist...
				{
					ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_5x5_Galileo_Ferry_1"));
					ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_5x5_Galileo_Ferry_2"));
					ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_5x5_Galileo_Ferry_3"));
					ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_5x5_Galileo_Ferry_4"));
					ShowFoundation(FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.LF_5x5_Galileo_Ferry_5"));
				}
			}

			auto PlaylistToUse = GetPlaylistToUse();

			if (PlaylistToUse)
			{
				static auto AdditionalLevelsOffset = PlaylistToUse->GetOffset("AdditionalLevels", false);

				if (AdditionalLevelsOffset != -1)
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

	// if (!Globals::bCreative)
	{
		static auto FortPlayerStartWarmupClass = Globals::bCreative ? FindObject<UClass>("/Script/FortniteGame.FortPlayerStartCreative") : FindObject<UClass>("/Script/FortniteGame.FortPlayerStartWarmup");
		TArray<AActor*> Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortPlayerStartWarmupClass);

		int ActorsNum = Actors.Num();

		Actors.Free();

		if (ActorsNum == 0)
			return false;
	}

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

		float TimeSeconds = UGameplayStatics::GetTimeSeconds(GetWorld());

		LOG_INFO(LogDev, "Initializing!");
		LOG_INFO(LogDev, "GameMode 0x{:x}", __int64(GameMode));

		GameState->Get<float>("WarmupCountdownEndTime") = TimeSeconds + Duration;
		GameMode->Get<float>("WarmupCountdownDuration") = Duration;

		GameState->Get<float>("WarmupCountdownStartTime") = TimeSeconds;
		GameMode->Get<float>("WarmupEarlyCountdownDuration") = EarlyDuration;

		static auto GameSessionOffset = GameMode->GetOffset("GameSession");
		auto GameSession = GameMode->Get<AActor*>(GameSessionOffset);
		static auto MaxPlayersOffset = GameSession->GetOffset("MaxPlayers");

		GameSession->Get<int>(MaxPlayersOffset) = 100;

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
					LOG_INFO(LogDev, "ReadyToStartMatch Return Address: 0x{:x}", __int64(_ReturnAddress()) - __int64(GetModuleHandleW(0)));
					return true;
				}
			}
		}
	}

	return Athena_ReadyToStartMatchOriginal(GameMode);
}

int AFortGameModeAthena::Athena_PickTeamHook(AFortGameModeAthena* GameMode, uint8 preferredTeam, AActor* Controller)
{
	LOG_INFO(LogTeams, "PickTeam called!");

	auto GameState = Cast<AFortGameStateAthena>(GameMode->GetGameState());

	static auto CurrentPlaylistDataOffset = GameState->GetOffset("CurrentPlaylistData", false);

	auto Playlist = CurrentPlaylistDataOffset == -1 && Fortnite_Version < 6 ? nullptr : GameState->GetCurrentPlaylist();

	static int CurrentTeamMembers = 0; // bad
	static int Current = 3;

	static int LastNum = 1;

	if (AmountOfRestarts != LastNum)
	{
		LastNum = AmountOfRestarts;

		Current = 3;
		CurrentTeamMembers = 0;
	}

	// std::cout << "Dru!\n";

	if (!Playlist)
	{
		CurrentTeamMembers = 0;
		LOG_INFO(LogTeams, "Player is going on team {} with {} members (No Playlist).", Current, CurrentTeamMembers);
		CurrentTeamMembers++;
		return Current++;
	}

	static int NextTeamIndex = 3; // Playlist->Get<uint8>("DefaultFirstTeam"); // + 1?

	static int LastNum1 = 1;

	if (AmountOfRestarts != LastNum1)
	{
		LastNum1 = AmountOfRestarts;

		NextTeamIndex = 3; // Playlist->Get<uint8>("DefaultFirstTeam"); // + 1?
	}

	// std::cout << "CurrentTeamMembers: " << CurrentTeamMembers << '\n';

	static auto MaxSquadSizeOffset = Playlist->GetOffset("MaxSquadSize");

	if (CurrentTeamMembers >= Playlist->Get<int>(MaxSquadSizeOffset))
	{
		// std::cout << "Moving next team!\n";

		NextTeamIndex++;
		CurrentTeamMembers = 0;
	}

	LOG_INFO(LogTeams, "Player is going on team {} with {} members.", NextTeamIndex, CurrentTeamMembers);

	CurrentTeamMembers++;

	return NextTeamIndex;
}

void AFortGameModeAthena::Athena_HandleStartingNewPlayerHook(AFortGameModeAthena* GameMode, AActor* NewPlayerActor)
{
	if (NewPlayerActor == GetLocalPlayerController()) // we dont really need this but it also functions as a nullptr check usually
		return;

	auto GameState = GameMode->GetGameStateAthena();

	LOG_INFO(LogPlayer, "HandleStartingNewPlayer!");

	// if (Engine_Version < 427)
	{
		static int LastNum69 = 19451;

		if (LastNum69 != AmountOfRestarts)
		{
			LastNum69 = AmountOfRestarts;

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

				CurrentActor->K2_DestroyActor();
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

				CurrentActor->K2_DestroyActor();
			}

			// SpawnIsland_FloorLoot_Actors.Free();
			// BRIsland_FloorLoot_Actors.Free();
		}

		LOG_INFO(LogDev, "Spawned loot!");
	}

	if (Engine_Version >= 423 && Engine_Version < 426) // 423+ we need to spawn manually and vehicle sync doesn't work on >S13.
	{
		static int LastNum420 = 1;

		if (AmountOfRestarts != LastNum420)
		{
			LastNum420 = AmountOfRestarts;

			SpawnVehicles2();
		}
	}

	auto NewPlayer = (AFortPlayerControllerAthena*)NewPlayerActor;

	auto PlayerStateAthena = NewPlayer->GetPlayerStateAthena();

	if (!PlayerStateAthena)
		return Athena_HandleStartingNewPlayerOriginal(GameMode, NewPlayerActor);

	if (Globals::bNoMCP)
	{
		static auto CharacterPartsOffset = PlayerStateAthena->GetOffset("CharacterParts", false);
		static auto CustomCharacterPartsStruct = FindObject<UStruct>("/Script/FortniteGame.CustomCharacterParts");

		if (CharacterPartsOffset != -1) // && CustomCharacterPartsStruct)
		{
			auto CharacterParts = PlayerStateAthena->GetPtr<__int64>("CharacterParts");

			static auto PartsOffset = FindOffsetStruct("/Script/FortniteGame.CustomCharacterParts", "Parts", false);
			auto Parts = (UObject**)(__int64(CharacterParts) + PartsOffset); // UCustomCharacterPart* Parts[0x6]

			static auto CustomCharacterPartClass = FindObject<UClass>("/Script/FortniteGame.CustomCharacterPart");

			static auto headPart = LoadObject("/Game/Characters/CharacterParts/Female/Medium/Heads/F_Med_Head1.F_Med_Head1", CustomCharacterPartClass);
			static auto bodyPart = LoadObject("/Game/Characters/CharacterParts/Female/Medium/Bodies/F_Med_Soldier_01.F_Med_Soldier_01", CustomCharacterPartClass);
			static auto backpackPart = LoadObject("/Game/Characters/CharacterParts/Backpacks/NoBackpack.NoBackpack", CustomCharacterPartClass);

			Parts[(int)EFortCustomPartType::Head] = headPart;
			Parts[(int)EFortCustomPartType::Body] = bodyPart;
			Parts[(int)EFortCustomPartType::Backpack] = backpackPart;

			static auto OnRep_CharacterPartsFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerState.OnRep_CharacterParts");
			PlayerStateAthena->ProcessEvent(OnRep_CharacterPartsFn);
		}
	}

	static auto SquadIdOffset = PlayerStateAthena->GetOffset("SquadId", false);

	if (SquadIdOffset != -1)
		PlayerStateAthena->GetSquadId() = PlayerStateAthena->GetTeamIndex() - 2;

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

	LOG_INFO(LogDev, "Old ID: {}", PlayerStateAthena->GetWorldPlayerId());
	LOG_INFO(LogDev, "PlayerID: {}", PlayerStateAthena->GetPlayerID());

	// if (PlayerStateAthena->GetWorldPlayerId() == -1)
	{
		static int CurrentPlayerId = 1;
		// static auto PlayerIdOffset = PlayerStateAthena->GetOffset("PlayerId"); // Unable to find tf
		PlayerStateAthena->GetWorldPlayerId() = PlayerStateAthena->GetPlayerID(); // ++CurrentPlayerId; // PlayerStateAthena->Get<int>(PlayerIdOffset); // 
	}

	if (Globals::bAbilitiesEnabled)
	{
		static auto FortAbilitySetClass = FindObject<UClass>("/Script/FortniteGame.FortAbilitySet");

		static auto GameplayAbilitySet = Fortnite_Version >= 8.30 ? // LoadObject<UObject>(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") ? 
			LoadObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer", FortAbilitySetClass) :
			LoadObject(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer", FortAbilitySetClass);

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

	struct FUniqueNetIdReplExperimental
	{
		unsigned char ahh[0x0028];
	};

	/* FUniqueNetIdReplExperimental Bugha{}; */
	static auto UniqueIdOffset = PlayerStateAthena->GetOffset("UniqueId");
	auto PlayerStateUniqueId = PlayerStateAthena->GetPtr<FUniqueNetIdRepl>(UniqueIdOffset);

	{
		LOG_INFO(LogDev, "bruh");
		static auto GameMemberInfoArrayOffset = GameState->GetOffset("GameMemberInfoArray", false);

		// if (false)
		// if (GameMemberInfoArrayOffset != -1)
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
				CopyStruct(&((FGameMemberInfo*)GameMemberInfo)->MemberUniqueId, PlayerStateUniqueId, FUniqueNetIdRepl::GetSizeOfStruct());
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

			static auto CreativePlotLinkedVolumeOffset = NewPlayer->GetOffset("CreativePlotLinkedVolume");
			NewPlayer->Get<AFortVolume*>(CreativePlotLinkedVolumeOffset) = Portal->GetLinkedVolume();

			Portal->GetLinkedVolume()->GetVolumeState() = EVolumeState::Ready;

			static auto IslandPlayset = FindObject<UFortPlaysetItemDefinition>("/Game/Playsets/PID_Playset_60x60_Composed.PID_Playset_60x60_Composed");

			if (auto Volume = NewPlayer->Get<AFortVolume*>(CreativePlotLinkedVolumeOffset))
			{
				// if (IslandPlayset)
					// Volume->UpdateSize({ (float)IslandPlayset->Get<int>("SizeX"), (float)IslandPlayset->Get<int>("SizeY"), (float)IslandPlayset->Get<int>("SizeZ") });

				static auto FortLevelSaveComponentClass = FindObject<UClass>("/Script/FortniteGame.FortLevelSaveComponent");
				auto LevelSaveComponent = (UObject*)Volume->GetComponentByClass(FortLevelSaveComponentClass);

				if (LevelSaveComponent)
				{
					static auto AccountIdOfOwnerOffset = LevelSaveComponent->GetOffset("AccountIdOfOwner");
					CopyStruct(LevelSaveComponent->GetPtr<FUniqueNetIdReplExperimental>(AccountIdOfOwnerOffset), PlayerStateUniqueId, FUniqueNetIdRepl::GetSizeOfStruct());
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

	if (Fortnite_Version <= 2.5)
	{
		static auto QuickBarsOffset = NewPlayer->GetOffset("QuickBars", false);

		if (QuickBarsOffset != -1)
		{
			static auto FortQuickBarsClass = FindObject<UClass>("/Script/FortniteGame.FortQuickBars");
			NewPlayer->Get<AActor*>(QuickBarsOffset) = GetWorld()->SpawnActor<AActor>(FortQuickBarsClass);
			NewPlayer->Get<AActor*>(QuickBarsOffset)->SetOwner(NewPlayer);
		}
	}

	if (Engine_Version <= 420) // not sure if needed on s3-s4
	{
		static auto OverriddenBackpackSizeOffset = NewPlayer->GetOffset("OverriddenBackpackSize");
		NewPlayer->Get<int>(OverriddenBackpackSizeOffset) = 5;
	}

	return Athena_HandleStartingNewPlayerOriginal(GameMode, NewPlayerActor);
}

void AFortGameModeAthena::SetZoneToIndexHook(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK)
{
	LOG_INFO(LogDev, "OverridePhaseMaybeIDFK: {}", OverridePhaseMaybeIDFK);
	return SetZoneToIndexOriginal(GameModeAthena, OverridePhaseMaybeIDFK);
}
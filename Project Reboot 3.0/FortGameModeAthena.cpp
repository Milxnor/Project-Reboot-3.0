#include "FortGameModeAthena.h"

#include "reboot.h"
#include "NetSerialization.h"
#include "FortPlayerControllerAthena.h"
#include "GameplayStatics.h"
#include "KismetStringLibrary.h"
#include "SoftObjectPtr.h"
#include "FortPickup.h"
#include "FortLootPackage.h"
#include "BuildingContainer.h"

static bool bFirstPlayerJoined = false;

enum class EDynamicFoundationEnabledState : uint8_t
{
	Unknown = 0,
	Enabled = 1,
	Disabled = 2,
	EDynamicFoundationEnabledState_MAX = 3
};


// Enum FortniteGame.EDynamicFoundationType
enum class EDynamicFoundationType : uint8_t
{
	Static = 0,
	StartEnabled_Stationary = 1,
	StartEnabled_Dynamic = 2,
	StartDisabled = 3,
	EDynamicFoundationType_MAX = 4
};

void ShowFoundation(AActor* BuildingFoundation)
{
	SetBitfield(BuildingFoundation->GetPtr<PlaceholderBitfield>("bServerStreamedInLevel"), 2, true);

	static auto OnRep_ServerStreamedInLevelFn = FindObject<UFunction>("/Script/FortniteGame.BuildingFoundation.OnRep_ServerStreamedInLevel");
	BuildingFoundation->ProcessEvent(OnRep_ServerStreamedInLevelFn);

	static auto DynamicFoundationRepDataOffset = BuildingFoundation->GetOffset("DynamicFoundationRepData", false);

	if (DynamicFoundationRepDataOffset != 0)
	{
		auto DynamicFoundationRepData = BuildingFoundation->GetPtr(DynamicFoundationRepDataOffset);

		static auto EnabledStateOffset = FindOffsetStruct("/Script/FortniteGame.DynamicBuildingFoundationRepData", "EnabledState");

		*(EDynamicFoundationEnabledState*)(__int64(DynamicFoundationRepData) + EnabledStateOffset) = EDynamicFoundationEnabledState::Enabled;

		static auto OnRep_DynamicFoundationRepDataFn = FindObject<UFunction>("/Script/FortniteGame.BuildingFoundation.OnRep_DynamicFoundationRepData");
		BuildingFoundation->ProcessEvent(OnRep_DynamicFoundationRepDataFn);
	}

	static auto FoundationEnabledStateOffset = BuildingFoundation->GetOffset("FoundationEnabledState");
	BuildingFoundation->Get<EDynamicFoundationEnabledState>(FoundationEnabledStateOffset) = EDynamicFoundationEnabledState::Enabled;
}

static void StreamLevel(std::string LevelName, FVector Location = {})
{
	static auto BuildingFoundation3x3Class = FindObject<UClass>("/Script/FortniteGame.BuildingFoundation3x3");
	FTransform Transform{};
	Transform.Scale3D = { 1, 1, 1 };
	Transform.Translation = Location;
	auto BuildingFoundation = GetWorld()->SpawnActor<ABuildingSMActor>(BuildingFoundation3x3Class, Transform);

	BuildingFoundation->InitializeBuildingActor(BuildingFoundation, nullptr, false);

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

	return Playlist;
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

		GameMode->Get<int>("WarmupRequiredPlayerCount") = 1;	
		
		auto PlaylistToUse = GetPlaylistToUse();

		if (PlaylistToUse)
		{
			static auto AdditionalLevelsOffset = PlaylistToUse->GetOffset("AdditionalLevels");
			auto& AdditionalLevels = PlaylistToUse->Get<TArray<TSoftObjectPtr<UClass>>>(AdditionalLevelsOffset);

			for (int i = 0; i < AdditionalLevels.Num(); i++)
			{
				// auto World = Cast<UWorld>(Playlist->AdditionalLevels[i].Get());
				// StreamLevel(UKismetSystemLibrary::GetPathName(World->PersistentLevel).ToString());
				StreamLevel(AdditionalLevels.at(i).SoftObjectPtr.ObjectID.AssetPathName.ToString());
			}
		}
	}

	/* static auto FortPlayerStartWarmupClass = FindObject<UClass>("/Script/FortniteGame.FortPlayerStartWarmup");
	TArray<AActor*> Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortPlayerStartWarmupClass);

	int ActorsNum = Actors.Num();

	Actors.Free();

	if (ActorsNum == 0)
		return false; */

	/* static int LastNum4 = 1;

	if (AmountOfRestarts != LastNum4)
	{
		LastNum4 = AmountOfRestarts;

		auto Playlist = FindObject("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
		SetPlaylist(Playlist);
		LOG_INFO(LogLoading, "Set playlist!");
	} */

	static auto MapInfoOffset = GameState->GetOffset("MapInfo");
	auto MapInfo = GameState->Get(MapInfoOffset);
	
	if (!MapInfo)
		return false;

	static auto FlightInfosOffset = MapInfo->GetOffset("FlightInfos");

	// if (MapInfo->Get<TArray<__int64>>(FlightInfosOffset).ArrayNum <= 0)
		// return false;

	static int LastNum3 = 1;

	if (AmountOfRestarts != LastNum3)
	{
		LastNum3 = AmountOfRestarts;

		GetWorld()->Listen();
		SetBitfield(GameMode->GetPtr<PlaceholderBitfield>("bWorldIsReady"), 1, true);

		// GameState->OnRep_CurrentPlaylistInfo();

		// return false;
	}

	// if (GameState->GetPlayersLeft() < GameMode->Get<int>("WarmupRequiredPlayerCount"))
	// if (!bFirstPlayerJoined)
		// return false;

	static int LastNum = 1;

	if (AmountOfRestarts != LastNum)
	{
		LastNum = AmountOfRestarts;

		float Duration = 40.f;
		float EarlyDuration = Duration;

		float TimeSeconds = 35.f; // UGameplayStatics::GetTimeSeconds(GetWorld());

		if (Engine_Version >= 424)
		{
			GameState->GetGamePhase() = EAthenaGamePhase::Warmup;
			GameState->OnRep_GamePhase();
		}

		SetPlaylist(GetPlaylistToUse());

		GameState->Get<float>("WarmupCountdownEndTime") = TimeSeconds + Duration;
		GameMode->Get<float>("WarmupCountdownDuration") = Duration;

		GameState->Get<float>("WarmupCountdownStartTime") = TimeSeconds;
		GameMode->Get<float>("WarmupEarlyCountdownDuration") = EarlyDuration;

		LOG_INFO(LogDev, "Initialized!");
	}

	if (Engine_Version >= 424) // returning true is stripped on c2+
	{
		// if (GameState->GetPlayersLeft() >= GameMode->Get<int>("WarmupRequiredPlayerCount"))
		if (MapInfo->Get<TArray<__int64>>(FlightInfosOffset).ArrayNum <= 0)
			return true;
	}

	return Athena_ReadyToStartMatchOriginal(GameMode);
}

int AFortGameModeAthena::Athena_PickTeamHook(AFortGameModeAthena* GameMode, uint8 preferredTeam, AActor* Controller)
{
	static auto NextTeamIndex = 3;
	return ++NextTeamIndex;
}

enum class EFortCustomPartType : uint8_t // todo move
{
	Head = 0,
	Body = 1,
	Hat = 2,
	Backpack = 3,
	Charm = 4,
	Face = 5,
	NumTypes = 6,
	EFortCustomPartType_MAX = 7
};

void AFortGameModeAthena::Athena_HandleStartingNewPlayerHook(AFortGameModeAthena* GameMode, AActor* NewPlayerActor)
{
	auto SpawnIsland_FloorLoot = FindObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");
	auto BRIsland_FloorLoot = FindObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");

	TArray<AActor*> SpawnIsland_FloorLoot_Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnIsland_FloorLoot);

	TArray<AActor*> BRIsland_FloorLoot_Actors =	UGameplayStatics::GetAllActorsOfClass(GetWorld(), BRIsland_FloorLoot);

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

		std::vector<std::pair<UFortItemDefinition*, int>> LootDrops = PickLootDrops(SpawnIslandTierGroup, bPrintWarmup);

		if (bPrintWarmup)
		{
			std::cout << "\n\n";
		}

		if (LootDrops.size())
		{
			for (auto& LootDrop : LootDrops)
				AFortPickup::SpawnPickup(LootDrop.first, Location, LootDrop.second, SpawnFlag);
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

		std::vector<std::pair<UFortItemDefinition*, int>> LootDrops = PickLootDrops(BRIslandTierGroup, bPrint);

		if (bPrint)
			std::cout << "\n";

		if (LootDrops.size())
		{
			for (auto& LootDrop : LootDrops)
				AFortPickup::SpawnPickup(LootDrop.first, Location, LootDrop.second, SpawnFlag);
		}
	}

	auto GameState = GameMode->GetGameStateAthena();

	auto NewPlayer = (AFortPlayerControllerAthena*)NewPlayerActor;

	auto WorldInventory = NewPlayer->GetWorldInventory();

	static UFortItemDefinition* EditToolItemDefinition = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/EditTool.EditTool");
	static UFortItemDefinition* PickaxeDefinition = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
	static UFortItemDefinition* BuildingItemData_Wall = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall");
	static UFortItemDefinition* BuildingItemData_Floor = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor");
	static UFortItemDefinition* BuildingItemData_Stair_W = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W");
	static UFortItemDefinition* BuildingItemData_RoofS = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS");
	static UFortItemDefinition* WoodItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");

	WorldInventory->AddItem(EditToolItemDefinition, nullptr);
	WorldInventory->AddItem(PickaxeDefinition, nullptr);
	WorldInventory->AddItem(BuildingItemData_Wall, nullptr);
	WorldInventory->AddItem(BuildingItemData_Floor, nullptr);
	WorldInventory->AddItem(BuildingItemData_Stair_W, nullptr);
	WorldInventory->AddItem(BuildingItemData_RoofS, nullptr);
	WorldInventory->AddItem(WoodItemData, nullptr, 100);

	WorldInventory->Update(true);

	auto PlayerStateAthena = NewPlayer->GetPlayerStateAthena();

	static auto CharacterPartsOffset = PlayerStateAthena->GetOffset("CharacterParts", false);

	if (CharacterPartsOffset != 0)
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

	if (false)
	{
		static auto GameplayAbilitySet = LoadObject<UObject>(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") ? 
			LoadObject<UObject>(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer") :
			LoadObject<UObject>(L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer");

		// LOG_INFO(LogDev, "GameplayAbilitySet {}", __int64(GameplayAbilitySet));

		if (GameplayAbilitySet)
		{
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

	// if (false)
	// if (GameMemberInfoArrayOffset != 0)
	if (Engine_Version >= 423)
	{
		struct FUniqueNetIdRepl
		{
			unsigned char ahh[0x0028];
		};

		struct FGameMemberInfo : public FFastArraySerializerItem
		{
			unsigned char                                      SquadId;                                                  // 0x000C(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			unsigned char                                      TeamIndex;                                                // 0x000D(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			unsigned char                                      UnknownData00[0x2];                                       // 0x000E(0x0002) MISSED OFFSET
			FUniqueNetIdRepl                            MemberUniqueId;                                           // 0x0010(0x0028) (HasGetValueTypeHash, NativeAccessSpecifierPublic)
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
			GameMemberInfo->MemberUniqueId = PlayerStateAthena->Get<FUniqueNetIdRepl>("UniqueId");
		}

		static auto GameMemberInfoArray_MembersOffset = 0x0108;

		auto GameMemberInfoArray = GameState->GetPtr<FFastArraySerializer>(GameMemberInfoArrayOffset);

		((TArray<FGameMemberInfo>*)(__int64(GameMemberInfoArray) + GameMemberInfoArray_MembersOffset))->Add(*GameMemberInfo, GameMemberInfoStructSize);
		GameMemberInfoArray->MarkArrayDirty();
	}

	return Athena_HandleStartingNewPlayerOriginal(GameMode, NewPlayer);
}
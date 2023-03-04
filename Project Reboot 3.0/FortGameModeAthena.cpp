#include "FortGameModeAthena.h"

#include "reboot.h"
#include "NetSerialization.h"
#include "FortPlayerControllerAthena.h"
#include "GameplayStatics.h"

static bool bFirstPlayerJoined = false;

bool AFortGameModeAthena::Athena_ReadyToStartMatchHook(AFortGameModeAthena* GameMode)
{
	auto GameState = GameMode->GetGameStateAthena();

	auto SetPlaylist = [&GameState](UObject* Playlist) -> void {
		if (Fortnite_Version >= 6.10)
		{
			auto CurrentPlaylistInfo = GameState->GetPtr<FFastArraySerializer>("CurrentPlaylistInfo");

			static auto PlaylistReplicationKeyOffset = FindOffsetStruct("/Script/FortniteGame.PlaylistPropertyArray", "PlaylistReplicationKey");
			static auto BasePlaylistOffset = FindOffsetStruct("/Script/FortniteGame.PlaylistPropertyArray", "BasePlaylist");

			*(UObject**)(__int64(CurrentPlaylistInfo) + BasePlaylistOffset) = Playlist;
			// CurrentPlaylistInfo.OverridePlaylist = Playlist;

			(*(int*)(__int64(CurrentPlaylistInfo) + PlaylistReplicationKeyOffset))++;
			CurrentPlaylistInfo->MarkArrayDirty();

			GameState->OnRep_CurrentPlaylistInfo();
		}
		else
		{
			GameState->Get("CurrentPlaylistData") = Playlist;
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

		auto Playlist = FindObject("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
		// SetPlaylist(Playlist);

		GameMode->Get<int>("WarmupRequiredPlayerCount") = 1;

		// LOG_INFO(LogLoading, "Set playlist!");
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

		auto Playlist = FindObject("/Game/Athena/Playlists/Playground/Playlist_Playground.Playlist_Playground"); // FindObject("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
		SetPlaylist(Playlist);

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

void AFortGameModeAthena::Athena_HandleStartingNewPlayerHook(AFortGameModeAthena* GameMode, AActor* NewPlayerActor)
{
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

	// if (false)
	{
		static auto GameplayAbilitySet = FindObject<UObject>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer");

		static auto GameplayAbilitiesOffset = GameplayAbilitySet->GetOffset("GameplayAbilities");
		auto GameplayAbilities = GameplayAbilitySet->GetPtr<TArray<UClass*>>(GameplayAbilitiesOffset);

		for (int i = 0; i < GameplayAbilities->Num(); i++)
		{
			UClass* AbilityClass = GameplayAbilities->At(i);

			// LOG_INFO(LogDev, "AbilityClass {}", __int64(AbilityClass));

			if (!AbilityClass)
				continue;

			// LOG_INFO(LogDev, "AbilityClass Name {}", AbilityClass->GetFullName());

			auto DefaultAbility = AbilityClass->CreateDefaultObject();

			// LOG_INFO(LogDev, "DefaultAbility {}", __int64(DefaultAbility));
			// LOG_INFO(LogDev, "DefaultAbility Name {}", DefaultAbility->GetFullName());

			PlayerStateAthena->GetAbilitySystemComponent()->GiveAbilityEasy(AbilityClass);
		}
	}
	
	// if (false)
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

		static auto GameMemberInfoArrayOffset = GameState->GetOffset("GameMemberInfoArray");
		auto GameMemberInfoArray = GameState->GetPtr<FFastArraySerializer>(GameMemberInfoArrayOffset);

		((TArray<FGameMemberInfo>*)(__int64(GameMemberInfoArray) + GameMemberInfoArray_MembersOffset))->Add(*GameMemberInfo, GameMemberInfoStructSize);
		GameMemberInfoArray->MarkArrayDirty();
	}

	return Athena_HandleStartingNewPlayerOriginal(GameMode, NewPlayer);
}
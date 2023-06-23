#include "FortGameStateAthena.h"

#include "reboot.h"
#include "FortPlayerStateAthena.h"
#include "FortGameModeAthena.h"
#include "FortAthenaMutator.h"
#include "gui.h"
#include "LevelStreamingDynamic.h"

void AFortGameStateAthena::AddPlayerStateToGameMemberInfo(AFortPlayerStateAthena* PlayerState)
{
	static auto GameMemberInfoArrayOffset = this->GetOffset("GameMemberInfoArray", false);

	if (GameMemberInfoArrayOffset == -1)
		return;

	static auto UniqueIdOffset = PlayerState->GetOffset("UniqueId");
	auto PlayerStateUniqueId = PlayerState->GetPtr<FUniqueNetIdRepl>(UniqueIdOffset);

	struct FUniqueNetIdWrapper
	{
		unsigned char                                      UnknownData00[0x1];                                       // 0x0000(0x0001) MISSED OFFSET
	};

	struct FUniqueNetIdReplExperimental : public FUniqueNetIdWrapper
	{
		unsigned char                                      UnknownData00[0x17];                                      // 0x0001(0x0017) MISSED OFFSET
		TArray<unsigned char>                              ReplicationBytes;                                         // 0x0018(0x0010) (ZeroConstructor, Transient, Protected, NativeAccessSpecifierProtected)
	};

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

		*(uint8*)(__int64(GameMemberInfo) + GameMemberInfo_SquadIdOffset) = PlayerState->GetSquadId();
		*(uint8*)(__int64(GameMemberInfo) + GameMemberInfo_TeamIndexOffset) = PlayerState->GetTeamIndex();
		CopyStruct((void*)(__int64(GameMemberInfo) + GameMemberInfo_MemberUniqueIdOffset), PlayerStateUniqueId, UniqueIdSize);
	}
	else
	{
		((FGameMemberInfo*)GameMemberInfo)->SquadId = PlayerState->GetSquadId();
		((FGameMemberInfo*)GameMemberInfo)->TeamIndex = PlayerState->GetTeamIndex();
		((FGameMemberInfo*)GameMemberInfo)->MemberUniqueId = PlayerState->Get<FUniqueNetIdReplExperimental>(UniqueIdOffset);
		// ((FUniqueNetIdRepl*)&((FGameMemberInfo*)GameMemberInfo)->MemberUniqueId)->CopyFromAnotherUniqueId(PlayerStateUniqueId);
	}

	static auto GameMemberInfoArray_MembersOffset = FindOffsetStruct("/Script/FortniteGame.GameMemberInfoArray", "Members");
	auto GameMemberInfoArray = this->GetPtr<FFastArraySerializer>(GameMemberInfoArrayOffset);

	((TArray<FGameMemberInfo>*)(__int64(GameMemberInfoArray) + GameMemberInfoArray_MembersOffset))->AddPtr(
		(FGameMemberInfo*)GameMemberInfo, GameMemberInfoStructSize
	);

	GameMemberInfoArray->MarkArrayDirty();
}

void AFortGameStateAthena::SkipAircraft()
{
	// return UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"skipaircraft", nullptr);

	if (GetGamePhase() != EAthenaGamePhase::Aircraft)
		return;

	// this->bGameModeWillSkipAircraft = true;

	auto GetAircrafts = [&]() -> std::vector<AActor*>
	{
		static auto AircraftsOffset = this->GetOffset("Aircrafts", false);
		std::vector<AActor*> Aircrafts;

		if (AircraftsOffset == -1)
		{
			// GameState->Aircraft

			static auto FortAthenaAircraftClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAircraft");
			auto AllAircrafts = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortAthenaAircraftClass);

			for (int i = 0; i < AllAircrafts.Num(); i++)
			{
				Aircrafts.push_back(AllAircrafts.at(i));
			}

			AllAircrafts.Free();
		}
		else
		{
			const auto& GameStateAircrafts = this->Get<TArray<AActor*>>(AircraftsOffset);

			for (int i = 0; i < GameStateAircrafts.Num(); i++)
			{
				Aircrafts.push_back(GameStateAircrafts.at(i));
			}
		}

		return Aircrafts;
	};

	auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

	for (auto Aircraft : GetAircrafts())
	{
		// haha skunked we should do GetAircraft!!
		static auto OnAircraftExitedDropZoneFn = FindObject<UFunction>(L"/Script/FortniteGame.FortGameModeAthena.OnAircraftExitedDropZone");
		GameMode->ProcessEvent(OnAircraftExitedDropZoneFn, &Aircraft);
	}
}

TScriptInterface<UFortSafeZoneInterface> AFortGameStateAthena::GetSafeZoneInterface()
{
	int Offset = -1;

	if (Fortnite_Version == 10.40)
	{
		// Offset = 0xF60;
	}

	TScriptInterface<UFortSafeZoneInterface> ScriptInterface{};

	if (Offset != -1)
	{
		auto idk = (void*)(__int64(this) + Offset);

		UObject* ObjectPtr = reinterpret_cast<UObject* (*)(__int64)>(((UObject*)idk)->VFTable[0x1])(__int64(idk)); // not actually a uobject but its just how we can get vft

		if (ObjectPtr)
		{
			ScriptInterface.ObjectPointer = ObjectPtr;
			ScriptInterface.InterfacePointer = ObjectPtr->GetInterfaceAddress(UFortSafeZoneInterface::StaticClass());
		}
	}

	return ScriptInterface;
}

UFortPlaylistAthena*& AFortGameStateAthena::GetCurrentPlaylist()
{
	static auto CurrentPlaylistInfoOffset = GetOffset("CurrentPlaylistInfo", false);

	if (CurrentPlaylistInfoOffset == -1)
	{
		static auto CurrentPlaylistDataOffset = GetOffset("CurrentPlaylistData");
		return (Get<UFortPlaylistAthena*>(CurrentPlaylistDataOffset));
	}

	auto CurrentPlaylistInfo = this->GetPtr<FFastArraySerializer>(CurrentPlaylistInfoOffset);

	static auto BasePlaylistOffset = FindOffsetStruct("/Script/FortniteGame.PlaylistPropertyArray", "BasePlaylist");
	return (*(UFortPlaylistAthena**)(__int64(CurrentPlaylistInfo) + BasePlaylistOffset));
}

int AFortGameStateAthena::GetAircraftIndex(AFortPlayerState* PlayerState)
{
	// The function has a string in it but we can just remake lol

	auto PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerState);

	if (!PlayerStateAthena)
		return 0;

	auto CurrentPlaylist = GetCurrentPlaylist();
	
	if (!CurrentPlaylist)
		return 0;

	static auto AirCraftBehaviorOffset = GetOffset("AirCraftBehavior");

	if (Get<uint8_t>(AirCraftBehaviorOffset) != 1) // AirCraftBehavior != EAirCraftBehavior::OpposingAirCraftForEachTeam
		return 0;

	auto TeamIndex = PlayerStateAthena->GetTeamIndex();
	int idfkwhatthisisimguessing = TeamIndex;

	static auto DefaultFirstTeamOffset = CurrentPlaylist->GetOffset("DefaultFirstTeam");
	auto DefaultFirstTeam = CurrentPlaylist->Get<int>(DefaultFirstTeamOffset);

	return TeamIndex - idfkwhatthisisimguessing;
}

bool AFortGameStateAthena::IsPlayerBuildableClass(UClass* Class)
{
	return true;

	static auto AllPlayerBuildableClassesOffset = GetOffset("AllPlayerBuildableClasses", false);

	if (AllPlayerBuildableClassesOffset == -1) // this is invalid in like s6 and stuff we need to find a better way to do this
		return true;

	auto& AllPlayerBuildableClasses = Get<TArray<UClass*>>(AllPlayerBuildableClassesOffset);

	for (int j = 0; j < AllPlayerBuildableClasses.Num(); j++)
	{
		auto CurrentPlayerBuildableClass = AllPlayerBuildableClasses.at(j);

		// LOG_INFO(LogDev, "CurrentPlayerBuildableClass: {}", CurrentPlayerBuildableClass->GetFullName());

		if (CurrentPlayerBuildableClass == Class)
			return true;
	}

	return false;

	// I don't know why but I think these are empty

	auto PlayerBuildableClasses = GetPlayerBuildableClasses();

	int ArraySize = 4 - 1;

	for (int i = 0; i < ArraySize; i++)
	{
		auto CurrentPlayerBuildableClassesArray = PlayerBuildableClasses[i].BuildingClasses;

		for (int j = 0; j < CurrentPlayerBuildableClassesArray.Num(); j++)
		{
			auto CurrentPlayerBuildableClass = CurrentPlayerBuildableClassesArray.at(j);

			LOG_INFO(LogDev, "CurrentPlayerBuildableClass: {}", CurrentPlayerBuildableClass->GetFullName());

			if (CurrentPlayerBuildableClass == Class)
				return true;
		}
	}

	return false;
}

bool AFortGameStateAthena::IsRespawningAllowed(AFortPlayerState* PlayerState)
{
	auto GameModeAthena = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
	static auto IsRespawningAllowedFn = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateZone.IsRespawningAllowed");

	// LOG_INFO(LogDev, "IsRespawningAllowedFn: {}", __int64(IsRespawningAllowedFn));

	if (!IsRespawningAllowedFn)
	{
		static auto CurrentPlaylistDataOffset = GetOffset("CurrentPlaylistData", false);
		auto CurrentPlaylist = CurrentPlaylistDataOffset == -1 && Fortnite_Version < 6 ? nullptr : GetCurrentPlaylist();

		if (!CurrentPlaylist)
			return false;

		static auto RespawnTypeOffset = CurrentPlaylist->GetOffset("RespawnType");

		if (RespawnTypeOffset == -1)
			return false;

		auto& RespawnType = CurrentPlaylist->Get<uint8_t>(RespawnTypeOffset);
		// LOG_INFO(LogDev, "RespawnType: {}", (int)RespawnType);

		if (RespawnType == 1)
			return true;
		
		if (RespawnType == 2) // InfiniteRespawnExceptStorm
		{
			static auto SafeZoneIndicatorOffset = GameModeAthena->GetOffset("SafeZoneIndicator");
			auto SafeZoneIndicator = GameModeAthena->Get<AActor*>(SafeZoneIndicatorOffset);

			if (!SafeZoneIndicator)
				return true;

			/*
			
			10.40

			bool __fastcall sub_7FF68F5A83A0(__int64 SafeZoneIndicator, float *a2)
			{
			  __m128 v2; // xmm1
			  float v3; // xmm2_4

			  v2 = *(*(SafeZoneIndicator + 928) + 464i64);
			  v3 = _mm_shuffle_ps(v2, v2, 85).m128_f32[0];
			  return (*(SafeZoneIndicator + 924) * *(SafeZoneIndicator + 924)) >= (((v3 - a2[1]) * (v3 - a2[1]))
																				 + ((v2.m128_f32[0] - *a2) * (v2.m128_f32[0] - *a2)));
			}

			If this returns true, then return true

			*/

			return true; // Do this until we implement ^^
		}

		return false;
	}

	struct { AFortPlayerState* PlayerState; bool ReturnValue; } AFortGameStateZone_IsRespawningAllowed_Params{PlayerState};
	this->ProcessEvent(IsRespawningAllowedFn, &AFortGameStateZone_IsRespawningAllowed_Params);

	return AFortGameStateZone_IsRespawningAllowed_Params.ReturnValue;
}

void AFortGameStateAthena::OnRep_GamePhase()
{
	EAthenaGamePhase OldGamePhase = GetGamePhase();

	static auto OnRep_GamePhase = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateAthena.OnRep_GamePhase");
	this->ProcessEvent(OnRep_GamePhase, &OldGamePhase);
}

void AFortGameStateAthena::OnRep_CurrentPlaylistInfo()
{
	static auto OnRep_CurrentPlaylistData = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateAthena.OnRep_CurrentPlaylistData");

	if (OnRep_CurrentPlaylistData)
	{
		this->ProcessEvent(OnRep_CurrentPlaylistData);
	}
	else
	{
		static auto OnRep_CurrentPlaylistInfo = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateAthena.OnRep_CurrentPlaylistInfo");

		if (OnRep_CurrentPlaylistInfo)
			this->ProcessEvent(OnRep_CurrentPlaylistInfo);
	}
}

void AFortGameStateAthena::OnRep_PlayersLeft()
{
	static auto OnRep_PlayersLeftFn = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateAthena.OnRep_PlayersLeft");

	if (!OnRep_PlayersLeftFn)
		return;

	this->ProcessEvent(OnRep_PlayersLeftFn);
}

TeamsArrayContainer* AFortGameStateAthena::GetTeamsArrayContainer()
{
	if (true)
		return nullptr;

	if (Fortnite_Version < 8.0) // I'm pretty sure it got added on 7.40 but idk if it is structured differently.
		return nullptr;

	static auto FriendlyFireTypeOffset = GetOffset("FriendlyFireType");
	static int Offset = -1;

	if (Offset == -1)
	{
		static int IncreaseBy = Engine_Version >= 424 ? 0x25 : 0x5;
		Offset = FriendlyFireTypeOffset + IncreaseBy;
	}

	return Offset != -1 ? (TeamsArrayContainer*)(__int64(this) + Offset) : nullptr;
}

void AFortGameStateAthena::AddToAdditionalPlaylistLevelsStreamed(const FName& Name, bool bServerOnly)
{
	auto NameStr = Name.ToString();
	auto NameWStr = std::wstring(NameStr.begin(), NameStr.end());

	if (true)
	{
		StreamLevel(Name.ToString()); // skunke bozo (I didn't test the next code too much soo)
	}
	else
	{
		static auto AdditionalPlaylistLevelsStreamedOffset = this->GetOffset("AdditionalPlaylistLevelsStreamed", false);

		if (!FAdditionalLevelStreamed::GetStruct())
		{
			auto& AdditionalPlaylistLevelsStreamed = this->Get<TArray<FName>>(AdditionalPlaylistLevelsStreamedOffset);
			AdditionalPlaylistLevelsStreamed.Add(Name);
		}
		else
		{
			auto& AdditionalPlaylistLevelsStreamed = this->Get<TArray<FAdditionalLevelStreamed>>(AdditionalPlaylistLevelsStreamedOffset);
			auto NewLevelStreamed = Alloc<FAdditionalLevelStreamed>(FAdditionalLevelStreamed::GetStructSize());
			NewLevelStreamed->GetLevelName() = Name;
			NewLevelStreamed->IsServerOnly() = bServerOnly;

			AdditionalPlaylistLevelsStreamed.AddPtr(NewLevelStreamed, FAdditionalLevelStreamed::GetStructSize());
		}
	}
}

UClass* AFortGameStateAthena::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortGameStateAthena");
	return Class;
}
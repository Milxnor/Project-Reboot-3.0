#pragma once

#include "GameState.h"
#include "FortPlayerStateAthena.h"
#include "FortPlaylistAthena.h"
#include "BuildingStructuralSupportSystem.h"
#include "ScriptInterface.h"
#include "Interface.h"
#include "FortAthenaMapInfo.h"

enum class EAthenaGamePhaseStep : uint8_t // idk if this changes
{
	None = 0,
	Setup = 1,
	Warmup = 2,
	GetReady = 3,
	BusLocked = 4,
	BusFlying = 5,
	StormForming = 6,
	StormHolding = 7,
	StormShrinking = 8,
	Countdown = 9,
	FinalCountdown = 10,
	EndGame = 11,
	Count = 12,
	EAthenaGamePhaseStep_MAX = 13
};

enum class EAthenaGamePhase : uint8_t
{
	None = 0,
	Setup = 1,
	Warmup = 2,
	Aircraft = 3,
	SafeZones = 4,
	EndGame = 5,
	Count = 6,
	EAthenaGamePhase_MAX = 7
};

class UFortSafeZoneInterface : public UInterface
{
public:
	static UClass* StaticClass()
	{
		static auto Struct = FindObject<UClass>(L"/Script/FortniteGame.FortSafeZoneInterface");
		return Struct;
	}
};

struct TeamsArrayContainer // THANK ANDROIDDD!!!!
{
	TArray<TArray<TWeakObjectPtr<AFortPlayerStateAthena>>> TeamsArray; // 13D0
	TArray<int> TeamIdk1; // 13E0
	TArray<int> TeamIndexesArray; // 13F0

	uintptr_t idfk; //(or 2 ints) // 1400

	TArray<TArray<TWeakObjectPtr<AFortPlayerStateAthena>>> SquadsArray; // Index = SquadId // 1408
	TArray<int> SquadIdk1; // 1418
	TArray<int> SquadIdsArray; // 0x1428
};

struct FPlayerBuildableClassContainer
{
	TArray<UClass*>                              BuildingClasses;                                          // 0x0000(0x0010) (ZeroConstructor, Transient, UObjectWrapper, NativeAccessSpecifierPublic)
};

struct FAdditionalLevelStreamed
{
public:
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.AdditionalLevelStreamed");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	FName& GetLevelName()
	{
		static auto LevelNameOffset = FindOffsetStruct("/Script/FortniteGame.AdditionalLevelStreamed", "LevelName");
		return *(FName*)(__int64(this) + LevelNameOffset);
	}
	
	bool& IsServerOnly()
	{
		static auto bIsServerOnlyOffset = FindOffsetStruct("/Script/FortniteGame.AdditionalLevelStreamed", "bIsServerOnly");
		return *(bool*)(__int64(this) + bIsServerOnlyOffset);
	}
};

class AFortGameStateAthena : public AGameState
{
public:
	int& GetPlayersLeft()
	{
		static auto PlayersLeftOffset = GetOffset("PlayersLeft");
		return Get<int>(PlayersLeftOffset);
	}

	bool& IsSafeZonePaused()
	{
		static auto bSafeZonePausedOffset = this->GetOffset("bSafeZonePaused");
		return this->Get<bool>(bSafeZonePausedOffset);
	}

	int& GetWorldLevel() // Actually in AFortGameState
	{
		static auto WorldLevelOffset = GetOffset("WorldLevel");
		return Get<int>(WorldLevelOffset);
	}

	EAthenaGamePhase& GetGamePhase()
	{
		static auto GamePhaseOffset = GetOffset("GamePhase");
		return Get<EAthenaGamePhase>(GamePhaseOffset);
	}

	UBuildingStructuralSupportSystem* GetStructuralSupportSystem() // actually in FortGameModeZone
	{
		static auto StructuralSupportSystemOffset = GetOffset("StructuralSupportSystem");
		return Get<UBuildingStructuralSupportSystem*>(StructuralSupportSystemOffset);
	}

	FPlayerBuildableClassContainer*& GetPlayerBuildableClasses()
	{
		static auto PlayerBuildableClassesOffset = GetOffset("PlayerBuildableClasses");
		return Get<FPlayerBuildableClassContainer*>(PlayerBuildableClassesOffset);
	}

	AFortAthenaMapInfo*& GetMapInfo()
	{
		static auto MapInfoOffset = GetOffset("MapInfo");
		return Get<AFortAthenaMapInfo*>(MapInfoOffset);
	}

	bool IsResurrectionEnabled(AFortPlayerPawn* PlayerPawn)
	{
		static auto IsResurrectionEnabledFn = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateAthena.IsResurrectionEnabled");
		struct { AFortPlayerPawn* PlayerPawn; bool Ret; } Params{PlayerPawn};
		this->ProcessEvent(IsResurrectionEnabledFn, &Params);
		return Params.Ret;
	}

	EAthenaGamePhaseStep& GetGamePhaseStep()
	{
		static auto GamePhaseStepOffset = GetOffset("GamePhaseStep");
		return Get<EAthenaGamePhaseStep>(GamePhaseStepOffset);
	}

	UFortPlaylistAthena*& GetCurrentPlaylist();
	TScriptInterface<UFortSafeZoneInterface> GetSafeZoneInterface();

	void AddPlayerStateToGameMemberInfo(class AFortPlayerStateAthena* PlayerState);
	void SkipAircraft();

	int GetAircraftIndex(AFortPlayerState* PlayerState);
	bool IsRespawningAllowed(AFortPlayerState* PlayerState); // actually in zone
	bool IsPlayerBuildableClass(UClass* Class);
	void OnRep_GamePhase();
	void OnRep_CurrentPlaylistInfo();
	void OnRep_PlayersLeft();
	TeamsArrayContainer* GetTeamsArrayContainer();
	void AddToAdditionalPlaylistLevelsStreamed(const FName& Name, bool bServerOnly = false);

	static UClass* StaticClass();
};
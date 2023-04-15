#pragma once

#include "GameState.h"
#include "FortPlayerState.h"
#include "FortPlaylist.h"
#include "BuildingStructuralSupportSystem.h"

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

struct FPlayerBuildableClassContainer
{
	TArray<UClass*>                              BuildingClasses;                                          // 0x0000(0x0010) (ZeroConstructor, Transient, UObjectWrapper, NativeAccessSpecifierPublic)
};

class AFortGameStateAthena : public AGameState
{
public:
	int& GetPlayersLeft()
	{
		static auto PlayersLeftOffset = GetOffset("PlayersLeft");
		return Get<int>(PlayersLeftOffset);
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

	UFortPlaylist*& GetCurrentPlaylist();

	// void AddPlayerStateToGameMemberInfo(class AFortPlayerStateAthena* PlayerState);

	int GetAircraftIndex(AFortPlayerState* PlayerState);
	bool IsRespawningAllowed(AFortPlayerState* PlayerState); // actually in zone
	bool IsPlayerBuildableClass(UClass* Class);
	void OnRep_GamePhase();
	void OnRep_CurrentPlaylistInfo();
};
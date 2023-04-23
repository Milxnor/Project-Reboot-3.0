#pragma once

#include "GameState.h"
#include "FortPlayerState.h"
#include "FortPlaylist.h"
#include "BuildingStructuralSupportSystem.h"
#include "ScriptInterface.h"
#include "Interface.h"

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
		static auto Struct = FindObject<UClass>("/Script/FortniteGame.FortSafeZoneInterface");
		return Struct;
	}
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
	TScriptInterface<UFortSafeZoneInterface> GetSafeZoneInterface();

	// void AddPlayerStateToGameMemberInfo(class AFortPlayerStateAthena* PlayerState);

	int GetAircraftIndex(AFortPlayerState* PlayerState);
	bool IsRespawningAllowed(AFortPlayerState* PlayerState); // actually in zone
	bool IsPlayerBuildableClass(UClass* Class);
	void OnRep_GamePhase();
	void OnRep_CurrentPlaylistInfo();
};

static void* ConstructOnGamePhaseStepChangedParams(EAthenaGamePhaseStep GamePhaseStep)
{
	struct AFortAthenaAIBotController_OnGamePhaseStepChanged_Params
	{
		TScriptInterface<UFortSafeZoneInterface>     SafeZoneInterface;                                        // (ConstParm, Parm, OutParm, ZeroConstructor, ReferenceParm, IsPlainOldData, NoDestructor, UObjectWrapper, NativeAccessSpecifierPublic)
		EAthenaGamePhaseStep                               GamePhaseStep;                                            // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	};

	bool bHasSafeZoneInterfaceParam = Fortnite_Version >= 10; // idk what version

	AFortAthenaAIBotController_OnGamePhaseStepChanged_Params* Params = Alloc<AFortAthenaAIBotController_OnGamePhaseStepChanged_Params>();
	
	if (bHasSafeZoneInterfaceParam)
	{
		auto GameState = (AFortGameStateAthena*)GetWorld()->GetGameState();

		auto Interface = GameState->GetSafeZoneInterface();

		if (!Interface.ObjectPointer)
			return nullptr;

		Params->SafeZoneInterface = Interface;
		Params->GamePhaseStep = GamePhaseStep;
	}
	else
	{
		*(EAthenaGamePhaseStep*)(__int64(Params) + 0) = GamePhaseStep;
	}

	return Params;
}
#pragma once

#include "FortGameModePvPBase.h"
// #include "FortPlayerControllerAthena.h"
#include "FortGameStateAthena.h"
#include "KismetStringLibrary.h"
#include "reboot.h"
#include "BuildingSMActor.h"

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

struct FAircraftFlightInfo
{
	float& GetTimeTillDropStart()
	{
		static auto TimeTillDropStartOffset = FindOffsetStruct("/Script/FortniteGame.AircraftFlightInfo", "TimeTillDropStart");
		return *(float*)(__int64(this) + TimeTillDropStartOffset);
	}

	FVector& GetFlightStartLocation()
	{
		static auto FlightStartLocationOffset = FindOffsetStruct("/Script/FortniteGame.AircraftFlightInfo", "FlightStartLocation");
		return *(FVector*)(__int64(this) + FlightStartLocationOffset);
	}

	float& GetFlightSpeed()
	{
		static auto FlightSpeedOffset = FindOffsetStruct("/Script/FortniteGame.AircraftFlightInfo", "FlightSpeed");
		return *(float*)(__int64(this) + FlightSpeedOffset);
	}

	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.AircraftFlightInfo");
		return Struct;
	}

	static int GetStructSize()
	{
		return GetStruct()->GetPropertiesSize();
	}
};

static void ShowFoundation(UObject* BuildingFoundation)
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

static void StreamLevel(const std::string& LevelName, FVector Location = {})
{
	static auto BuildingFoundation3x3Class = FindObject<UClass>("/Script/FortniteGame.BuildingFoundation3x3");
	FTransform Transform{};
	Transform.Scale3D = { 1, 1, 1 };
	Transform.Translation = Location;
	auto BuildingFoundation = GetWorld()->SpawnActor<ABuildingSMActor>(BuildingFoundation3x3Class, Transform);

	if (!BuildingFoundation)
	{
		LOG_ERROR(LogGame, "Failed to spawn BuildingFoundation for streaming!");
		return;
	}

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

class AFortGameModeAthena : public AFortGameModePvPBase
{
public:
	static inline bool (*Athena_ReadyToStartMatchOriginal)(AFortGameModeAthena* GameMode);
	static inline void (*Athena_HandleStartingNewPlayerOriginal)(AFortGameModeAthena* GameMode, AActor* NewPlayer);
	static inline void (*SetZoneToIndexOriginal)(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK);

	AFortGameStateAthena* GetGameStateAthena()
	{
		return (AFortGameStateAthena*)GetGameState();
	}

	FName RedirectLootTier(const FName& LootTier);
	UClass* GetVehicleClassOverride(UClass* DefaultClass);

	static bool Athena_ReadyToStartMatchHook(AFortGameModeAthena* GameMode);
	static int Athena_PickTeamHook(AFortGameModeAthena* GameMode, uint8 preferredTeam, AActor* Controller);
	static void Athena_HandleStartingNewPlayerHook(AFortGameModeAthena* GameMode, AActor* NewPlayerActor);
	static void SetZoneToIndexHook(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK);
};
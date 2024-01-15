#pragma once

#include "reboot.h"
#include "GameplayStatics.h"
#include "FortGameStateAthena.h"

namespace Calendar
{
	static inline bool HasSnowModification()
	{
		return Fortnite_Version == 7.10 || Fortnite_Version == 7.30 || Fortnite_Version == 11.31 || Fortnite_Version == 15.10 || Fortnite_Version == 19.01 || Fortnite_Version == 19.10;
	}

	static inline UObject* GetSnowSetup()
	{
		auto Class = FindObject<UClass>(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C") ? FindObject<UClass>(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C") :
			FindObject<UClass>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C");

		auto S19Class = FindObject(L"/SpecialSurfaceCoverage/Maps/SpecialSurfaceCoverage_Artemis_Terrain_LS_Parent_Overlay.SpecialSurfaceCoverage_Artemis_Terrain_LS_Parent_Overlay.PersistentLevel.BP_Artemis_S19Progression_C_0");

		auto Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), Class);

		return Actors.Num() > 0 ? Actors.at(0) : S19Class ? S19Class : nullptr;
	}

	static inline float GetFullSnowMapValue()
	{
		if (Fortnite_Version == 7.10 || Fortnite_Version == 7.30)
		{
			std::vector<std::pair<float, float>> TimeAndValues = { { 0, 1.2f}, { 0.68104035f, 4.6893263f }, { 0.9632137f, 10.13335f }, { 1.0f, 15.0f } };
			// 1.2
			// 4.6893263
			// 10.13335
			// 15;
			return TimeAndValues[3].first;
		}
		else if (Fortnite_Version == 11.31 || Fortnite_Version == 15.10)
		{
			std::vector<std::pair<float, float>> TimeAndValues = { { 0, 0.0f }, { 0.5f, 0.35f }, { 0.75f, 0.5f }, { 1.0f, 1.0f } };
			// 0
			// 0.35
			// 0.5
			// 1
			return TimeAndValues[3].first;
		}
		else if (Fortnite_Version == 19.01 || Fortnite_Version == 19.10)
		{
			std::vector<int> Values = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

			return Values[6];
		}

		return -1;
	}

	static inline void EnableFog()
	{
		auto SnowSetup = GetSnowSetup();

		if (SnowSetup)
		{
			static auto Onready1 = FindObject<UFunction>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.OnReady_E426AA7F4F2319EA06FBA2B9905F0B24");
			static auto Onready2 = FindObject<UFunction>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.OnReady_0A511B314AE165C51798519FB84738B8");
			static auto refrehsdmap = FindObject<UFunction>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C:RefreshMapLocations");

			auto GameState = (AFortGameStateAthena*)GetWorld()->GetGameState();

			GET_PLAYLIST(GameState)

				struct { UObject* GameState; UObject* Playlist; FGameplayTagContainer PlaylistContextTags; } OnReadyParams{ GameState, CurrentPlaylist, FGameplayTagContainer() };

			SnowSetup->ProcessEvent(Onready1, &OnReadyParams);
			SnowSetup->ProcessEvent(Onready2, &OnReadyParams);
			SnowSetup->ProcessEvent(refrehsdmap); // needed?
		}
	}

	static inline void SetSnowfall(float NewValue) // idk bruh i think setsnow also does this math things who knows
	{
		static auto SetSnowfallFn = FindObject<UFunction>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.SetSnowFall");
		auto SnowSetup = GetSnowSetup();

		if (SetSnowfallFn && SnowSetup)
		{
			SnowSetup->ProcessEvent(SetSnowfallFn, &NewValue);
		}
	}

	static inline void SetSnow(float NewValue)
	{

		static auto SetSnowFn = FindObject<UFunction>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.SetSnow") ? FindObject<UFunction>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.SetSnow") :
			FindObject<UFunction>(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C.SetSnow") ? FindObject<UFunction>(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C.SetSnow") :
			FindObject<UFunction>("/SpecialSurfaceCoverage/Items/BP_Artemis_S19Progression.BP_Artemis_S19Progression_C.SetSnowProgressionPhase");

		auto SnowSetup = GetSnowSetup();

		LOG_INFO(LogDev, "SnowSetup: {}", SnowSetup->IsValidLowLevel() ? SnowSetup->GetFullName() : "BadRead");

		if (SnowSetup && SetSnowFn)
		{
			if (Engine_Version >= 500)
				NewValue = static_cast<int>(NewValue);

			SnowSetup->ProcessEvent(SetSnowFn, &NewValue);

			if (NewValue != -1 && Engine_Version >= 500)
			{
				auto UpdateSnowVisualsOnClientFn = FindObject<UFunction>("/SpecialSurfaceCoverage/Items/BP_Artemis_S19Progression.BP_Artemis_S19Progression_C.UpdateSnowVisualsOnClient");
				SnowSetup->ProcessEvent(UpdateSnowVisualsOnClientFn);
			}

			LOG_INFO(LogDev, "Called SetSnow!");
		}
		else
		{
			LOG_INFO(LogDev, "Failed TO FIND!");
		}
	}

	static inline void SetWaterLevel(int WaterLevel)
	{
		UObject* WL = FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.Apollo_WaterSetup_2");

		if (WL && WaterLevel != -1)
		{
			static auto SetWaterLevel = FindObject<UFunction>("/Game/Athena/Apollo/Environments/Blueprints/Apollo_WaterSetup.Apollo_WaterSetup_C.SetWaterLevel");
			static auto OnRep_CurrentWaterLevel = FindObject<UFunction>("/Game/Athena/Apollo/Environments/Blueprints/Apollo_WaterSetup.Apollo_WaterSetup_C.OnRep_CurrentWaterLevel");

			WL->ProcessEvent(SetWaterLevel, &WaterLevel);
			WL->ProcessEvent(OnRep_CurrentWaterLevel);
		}
	}

	static inline bool HasNYE()
	{
		return Fortnite_Version == 7.10 || Fortnite_Version == 11.31 || Fortnite_Version == 15.10 || Fortnite_Version == 19.00 || Fortnite_Version == 19.01;
	}

	static inline void StartNYE()
	{
		static auto NewYearTimer = FindObject<UObject>("/Game/Athena/Maps/Streaming/Athena_NYE_Celebration.Athena_NYE_Celebration.PersistentLevel.BP_NewYearTimer_2") ? FindObject<UObject>("/Game/Athena/Maps/Streaming/Athena_NYE_Celebration.Athena_NYE_Celebration.PersistentLevel.BP_NewYearTimer_2") :
			FindObject<UObject>("/NewYears/Content/Levels/Apollo_NYE_Celebration.Apollo_NYE_Celebration.PersistentLevel.BP_NewYearTimer_2") ? FindObject<UObject>("/NewYears/Content/Levels/Apollo_NYE_Celebration.Apollo_NYE_Celebration.PersistentLevel.BP_NewYearTimer_2") :
			FindObject<UObject>("/NewYears/Content/Levels/Artemis_NYE_Celebration.Artemis_NYE_Celebration.PersistentLevel.BP_NewYearTimer_2");

		LOG_INFO(LogDev, "NewYearTimer: {}", NewYearTimer->IsValidLowLevel() ? NewYearTimer->GetFullName() : "BadRead");

		static auto StartNYE = FindObject<UFunction>("/Game/Athena/Events/NewYear/BP_NewYearTimer.BP_NewYearTimer_C.startNYE") ? FindObject<UFunction>("/Game/Athena/Events/NewYear/BP_NewYearTimer.BP_NewYearTimer_C.startNYE") :
			FindObject<UFunction>("/NewYears/Blueprints/BP_NewYearTimer.BP_NewYearTimer_C.startNYE");

		LOG_INFO(LogDev, "StartNYE: {}", StartNYE->IsValidLowLevel() ? StartNYE->GetFullName() : "BadRead");

		if (NewYearTimer && StartNYE)
		{
			NewYearTimer->ProcessEvent(StartNYE, nullptr);
		}
	}
}
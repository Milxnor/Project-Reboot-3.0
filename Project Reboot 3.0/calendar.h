#pragma once

#include "reboot.h"
#include "GameplayStatics.h"
#include "FortGameStateAthena.h"

namespace Calendar
{
	static inline bool HasSnowModification()
	{
		return Fortnite_Version == 7.30 || Fortnite_Version == 11.31 || Fortnite_Version == 19.10;
	}

	static inline UObject* GetSnowSetup()
	{
		auto Class = FindObject<UClass>(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C");
		auto Actors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), Class);

		return Actors.Num() > 0 ? Actors.at(0) : nullptr;
	}

	static inline float GetFullSnowMapValue()
	{
		if (Fortnite_Version == 7.30)
		{
			std::vector<std::pair<float, float>> TimeAndValues = { { 0, 1.2f}, { 0.68104035f, 4.6893263f }, { 0.9632137f, 10.13335f }, { 1.0f, 15.0f } };
			// 1.2
			// 4.6893263
			// 10.13335
			// 15;
			return TimeAndValues[3].first;
		}

		return -1;
	}

	static inline void SetSnow(float NewValue)
	{
		static auto SetSnowFn = FindObject<UFunction>(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C.SetSnow");
		auto SnowSetup = GetSnowSetup();

		LOG_INFO(LogDev, "SnowSetup: {}", SnowSetup->IsValidLowLevel() ? SnowSetup->GetFullName() : "BadRead");

		if (SnowSetup)
		{
			static auto OnReady_347B1F4D45630C357605FCB417D749A3Fn = FindObject<UFunction>(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C.OnReady_347B1F4D45630C357605FCB417D749A3");
			auto GameState = GetWorld()->GetGameState();
			SnowSetup->ProcessEvent(OnReady_347B1F4D45630C357605FCB417D749A3Fn, &GameState);

			SnowSetup->ProcessEvent(SetSnowFn, &NewValue);
		}
	}

    static inline void SetWaterLevel(int WaterLevel)
    {
        // There was 7 water levels
        UObject* WL = FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.Apollo_WaterSetup_2"); // Is this 13.40 specific?

        if (WL)
        {
            static auto SetWaterLevel = FindObject<UFunction>("/Game/Athena/Apollo/Environments/Blueprints/Apollo_WaterSetup.Apollo_WaterSetup_C.SetWaterLevel");
            static auto OnRep_CurrentWaterLevel = FindObject<UFunction>("/Game/Athena/Apollo/Environments/Blueprints/Apollo_WaterSetup.Apollo_WaterSetup_C.OnRep_CurrentWaterLevel");

            WL->ProcessEvent(SetWaterLevel, &WaterLevel);
            WL->ProcessEvent(OnRep_CurrentWaterLevel);

            /* UObject* FoundationSetup = FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations:PersistentLevel.Apollo_WaterSetup_Foundations_2");
            if (FoundationSetup)
            {
                static auto SetWaterFoundations = FindObject<UFunction>("/Game/Athena/Apollo/Environments/Blueprints/Apollo_WaterSetup_Foundations.Apollo_WaterSetup_Foundations_C.SetWaterFoundations");
                FoundationSetup->ProcessEvent(SetWaterFoundations, &WaterLevel);
            } */
        }
    }
}
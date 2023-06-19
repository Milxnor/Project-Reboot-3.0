#pragma once

#include "reboot.h"
#include "GameplayStatics.h"
#include "FortGameStateAthena.h"

extern inline UObject** GetCalendarPlaylist()
{
	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	static auto CurrentPlaylistDataOffset = GameState->GetOffset("CurrentPlaylistData", false);

	// if (Fortnite_Version >= 6.10) // 6.00
	if (CurrentPlaylistDataOffset == 0)
	{
		static auto BasePlaylistOffset = FindOffsetStruct("ScriptStruct /Script/FortniteGame.PlaylistPropertyArray", ("BasePlaylist"));

		if (BasePlaylistOffset)
		{
			static auto CurrentPlaylistInfoOffset = GameState->GetOffset("CurrentPlaylistInfo");
			auto PlaylistInfo = (void*)(__int64(GameState) + CurrentPlaylistInfoOffset); // gameState->Member<void>(("CurrentPlaylistInfo"));

			auto BasePlaylist = (UObject**)(__int64(PlaylistInfo) + BasePlaylistOffset);// *gameState->Member<UObject>(("CurrentPlaylistInfo"))->Member<UObject*>(("BasePlaylist"), true);

			return BasePlaylist;
		}
	}
	else
	{
		auto PlaylistData = (UObject**)(__int64(GameState) + CurrentPlaylistDataOffset);

		return PlaylistData;
	}

	return nullptr;
}

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

	static inline void SetSnowIndex(int SnowIndex)
	{
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

		if (Fortnite_Version >= 19.10 && Fortnite_Version <= 19.40)
		{
			auto sjt9ase9i = FindObject("/SpecialSurfaceCoverage/Maps/SpecialSurfaceCoverage_Artemis_Terrain_LS_Parent_Overlay.SpecialSurfaceCoverage_Artemis_Terrain_LS_Parent_Overlay.PersistentLevel.BP_Artemis_S19Progression_C_0");

			// std::cout << "sjt9ase9i: " << sjt9ase9i << '\n';

			if (sjt9ase9i)
			{
				auto setprogr = FindObject<UFunction>("/SpecialSurfaceCoverage/Items/BP_Artemis_S19Progression.BP_Artemis_S19Progression_C.SetSnowProgressionPhase");
				sjt9ase9i->ProcessEvent(setprogr, &SnowIndex);

				auto agh = FindObject<UFunction>("/SpecialSurfaceCoverage/Items/BP_Artemis_S19Progression.BP_Artemis_S19Progression_C.UpdateSnowVisualsOnClient");
				sjt9ase9i->ProcessEvent(agh); // idk if this is needed
			}
		}

		if (Fortnite_Version >= 11.00 && Fortnite_Version <= 11.50)
		{
			auto agfag = FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.BP_ApolloSnowSetup_2");

			std::cout << "agfag: " << agfag << '\n';

			if (agfag)
			{
				struct { UObject* GameState; UObject* Playlist; FGameplayTagContainer PlaylistContextTags; } bbparms{ GameState, *GetCalendarPlaylist(),
					FGameplayTagContainer() };

				auto OnReady_E426AA7F4F2319EA06FBA2B9905F0B24 = FindObject<UFunction>("/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C.OnReady_E426AA7F4F2319EA06FBA2B9905F0B24");

				if (OnReady_E426AA7F4F2319EA06FBA2B9905F0B24)
					agfag->ProcessEvent(OnReady_E426AA7F4F2319EA06FBA2B9905F0B24, &GameState);

				auto OnReady_0A511B314AE165C51798519FB84738B8 = FindObject<UFunction>("/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C.OnReady_0A511B314AE165C51798519FB84738B8");

				if (OnReady_0A511B314AE165C51798519FB84738B8)
					agfag->ProcessEvent(OnReady_0A511B314AE165C51798519FB84738B8, &bbparms);

				auto age = FindObject<UFunction>("/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.RefreshMapLocations");
				agfag->ProcessEvent(age);
			}
		}

		if (Fortnite_Version >= 7.00 && Fortnite_Version <= 7.40)
		{
			auto snowsetup = FindObject("/Game/Athena/Maps/Athena_Terrain.Athena_Terrain.PersistentLevel.BP_SnowSetup_2");

			std::cout << "snowsteup: " << snowsetup << '\n';

			if (snowsetup)
			{
				auto OnReady_347B1F4D45630C357605FCB417D749A3 = FindObject<UFunction>("/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C.OnReady_347B1F4D45630C357605FCB417D749A3");

				if (OnReady_347B1F4D45630C357605FCB417D749A3)
					snowsetup->ProcessEvent(OnReady_347B1F4D45630C357605FCB417D749A3, &GameState);

				auto afga = FindObject<UFunction>("/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C.SetSnow");

				if (afga)
					snowsetup->ProcessEvent(afga, &SnowIndex);

				auto RefreshMapLocations = FindObject<UFunction>("/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C.RefreshMapLocations");

				if (RefreshMapLocations)
					snowsetup->ProcessEvent(RefreshMapLocations);
			}
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

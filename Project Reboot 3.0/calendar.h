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
		auto Class = FindObject<UClass>(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C") ? FindObject<UClass>(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C") :
			FindObject<UClass>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C");

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
		else if (Fortnite_Version == 11.31)
		{
			return 100;
		}

		return -1;
	}

	static inline void SetSnow(float NewValue)
	{
		static auto SetSnowFn = FindObject<UFunction>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.OnReady_0A511B314AE165C51798519FB84738B8") ? FindObject<UFunction>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.OnReady_0A511B314AE165C51798519FB84738B8") :
			FindObject<UFunction>(L"/Game/Athena/Environments/Landscape/Blueprints/BP_SnowSetup.BP_SnowSetup_C.SetSnow");
		auto SnowSetup = GetSnowSetup();

		LOG_INFO(LogDev, "SnowSetup: {}", SnowSetup->IsValidLowLevel() ? SnowSetup->GetFullName() : "BadRead");

		if (SnowSetup && SetSnowFn)
		{
			auto GameState = (AFortGameStateAthena*)GetWorld()->GetGameState();

			GET_PLAYLIST(GameState)
			struct { UObject* GameState; UObject* Playlist; FGameplayTagContainer PlaylistContextTags; } OnReadyParams{ GameState, CurrentPlaylist, FGameplayTagContainer()};

			UFunction* OnReadyFunc = FindObject<UFunction>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.OnReady_0A511B314AE165C51798519FB84738B8");

			LOG_INFO(LogDev, "Calling OnReady!");

			if (OnReadyFunc)
			{
				SnowSetup->ProcessEvent(OnReadyFunc, &OnReadyParams);
			}

			LOG_INFO(LogDev, "Called OnReady!");

			return;

			if (NewValue != -1)
			{
				static auto SnowAmountOffset = SnowSetup->GetOffset("SnowAmount");
				SnowSetup->Get<float>(SnowAmountOffset) = NewValue;

				static auto OnRep_Snow_AmountFn = FindObject<UFunction>("/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.OnRep_Snow_Amount");
				SnowSetup->ProcessEvent(OnRep_Snow_AmountFn);

				// SnowSetup->ProcessEvent(SetSnowFn, &NewValue);
			}

			auto SetFallingSnowFn = FindObject<UFunction>("/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.SetFallingSnow");

			LOG_INFO(LogDev, "Called SetSnow!");

			static auto ada = FindObject<UFunction>(L"/Game/Athena/Apollo/Environments/Blueprints/CalendarEvents/BP_ApolloSnowSetup.BP_ApolloSnowSetup_C.SetFullSnowEd");
			SnowSetup->ProcessEvent(ada);
		}
	}
}
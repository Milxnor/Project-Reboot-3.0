#pragma once

#include <map>

#include "Object.h"
#include "reboot.h"
#include "GameplayStatics.h"

struct Event
{
	std::string EventDisplayName;
	std::string LoaderClass;
	std::string LoaderFunction;
	__int64 AdditionalLoaderParams;
	std::vector<std::pair<bool, std::string>> OnReadyFunctions;
	std::vector<std::pair<std::pair<bool, std::string>, __int64>> StartEventFunctions;
	std::string ScriptingClass;
	std::string PlaylistName;
	double Version;
};

static inline std::vector<Event> Events = 
{
	Event
	(
		"Astronomical", 
		"/CycloneJerky/Gameplay/BP_Jerky_Loader.BP_Jerky_Loader_C",
		// "/CycloneJerky/Gameplay/BP_Jerky_Loader.BP_Jerky_Loader_C.LoadJerkyLevel", 
		"",
		1,
		{ 
			{ 
				false, 
				"/CycloneJerky/Gameplay/BP_Jerky_Scripting.BP_Jerky_Scripting_C.OnReady_093B6E664C060611B28F79B5E7052A39" 
			},
			{
				true,
				"/CycloneJerky/Gameplay/BP_Jerky_Loader.BP_Jerky_Loader_C.OnReady_7FE9744D479411040654F5886C078D08" 
			}
		},
		{ 
			{ 
				/* {
					false, 
					"/CycloneJerky/Gameplay/BP_Jerky_Scripting.BP_Jerky_Scripting_C.startevent"
				}, */
				{
					true,
					"/CycloneJerky/Gameplay/BP_Jerky_Loader.BP_Jerky_Loader_C.startevent"
				},

				0
			}
		},

		"/CycloneJerky/Gameplay/BP_Jerky_Scripting.BP_Jerky_Scripting_C", 
		"/Game/Athena/Playlists/Music/Playlist_Music_High.Playlist_Music_High", 
		12.41
	),
	Event
	(
		"Device",
		"/Fritter/BP_Fritter_Loader.BP_Fritter_Loader_C",
		// "/CycloneJerky/Gameplay/BP_Jerky_Loader.BP_Jerky_Loader_C.LoadJerkyLevel", 
		"",
		1,
		{
			{
				false,
				"/Fritter/BP_Fritter_Script.BP_Fritter_Script_C.OnReady_ACE66C28499BF8A59B3D88A981DDEF41"
			},
			{
				true,
				"/Fritter/BP_Fritter_Loader.BP_Fritter_Loader_C.OnReady_1216203B4B63E3DFA03042A62380A674"
			}
		},
		{
			{
				/* {
					false,
					"/Fritter/BP_Fritter_Loader.BP_Fritter_Loader_C.startevent"
				}, */
				{
					true,
					"/Fritter/BP_Fritter_Loader.BP_Fritter_Loader_C.startevent"
				},

				0
			}
		},

		"/Fritter/BP_Fritter_Script.BP_Fritter_Script_C",
		"/Game/Athena/Playlists/Fritter/Playlist_Fritter_High.Playlist_Fritter_High",
		12.61
	),
	Event
	(
		"Marshmello",
		"",
		"",
		1,
		{
			{
				false,
				"/Game/Athena/Environments/Festivus/Blueprints/BP_FestivusManager.BP_FestivusManager_C.OnReady_EE7676604ADFD92D7B2972AC0ABD4BB8"
			}
		},
		{
			{
				{
					false,
					"/Game/Athena/Environments/Festivus/Blueprints/BP_FestivusManager.BP_FestivusManager_C.PlayConcert"
				},

				0
			}
		},

		"/Game/Athena/Environments/Festivus/Blueprints/BP_FestivusManager.BP_FestivusManager_C",
		"/Game/Athena/Playlists/Music/Playlist_Music_High.Playlist_Music_High",
		7.30
	)
};

static inline UObject* GetEventPlaylist()
{
	for (auto& CurrentEvent : Events)
	{
		if (CurrentEvent.Version == Fortnite_Version)
			return FindObject(CurrentEvent.PlaylistName, nullptr, ANY_PACKAGE);
	}

	return nullptr;
}

static inline UObject* GetEventScripting()
{
	Event OurEvent;

	for (auto& CurrentEvent : Events)
	{
		if (CurrentEvent.Version == Fortnite_Version)
		{
			OurEvent = CurrentEvent;
			break;
		}
	}

	if (!OurEvent.Version)
		return nullptr;

	auto ScriptingClass = FindObject<UClass>(OurEvent.ScriptingClass);

	if (!ScriptingClass)
	{
		// LOG_ERROR(LogEvent, "Failed to find ScriptingClass!");
		return nullptr;
	}

	auto AllScripters = UGameplayStatics::GetAllActorsOfClass(GetWorld(), ScriptingClass);

	if (AllScripters.size() <= 0)
	{
		// LOG_ERROR(LogEvent, "Failed to find any scripters!");
		return nullptr;
	}

	return AllScripters.at(0);
}

static inline UObject* GetEventLoader()
{
	Event OurEvent;

	for (auto& CurrentEvent : Events)
	{
		if (CurrentEvent.Version == Fortnite_Version)
		{
			OurEvent = CurrentEvent;
			break;
		}
	}

	if (!OurEvent.Version)
		return nullptr;

	auto LoaderClass = FindObject<UClass>(OurEvent.LoaderClass);

	if (!LoaderClass)
	{
		LOG_ERROR(LogEvent, "Failed to find LoaderClass!");
		return nullptr;
	}

	auto AllLoaders = UGameplayStatics::GetAllActorsOfClass(GetWorld(), LoaderClass);

	if (AllLoaders.size() <= 0)
	{
		// LOG_ERROR(LogEvent, "Failed to find any loaders!");
		return nullptr;
	}

	return AllLoaders.at(0);
}

static inline std::string GetEventName()
{
	for (auto& CurrentEvent : Events)
	{
		if (CurrentEvent.Version == Fortnite_Version)
			return CurrentEvent.EventDisplayName;
	}

	return "";
}

static inline void LoadEvent(bool* bWereAllSuccessful = nullptr)
{
	if (bWereAllSuccessful)
		*bWereAllSuccessful = false;

	Event OurEvent;

	for (auto& CurrentEvent : Events)
	{
		if (CurrentEvent.Version == Fortnite_Version)
		{
			OurEvent = CurrentEvent;
			break;
		}
	}

	if (!OurEvent.Version)
		return;

	if (bWereAllSuccessful)
		*bWereAllSuccessful = true;

	auto LoaderFunction = FindObject<UFunction>(OurEvent.LoaderFunction);

	if (!LoaderFunction)
	{
		LOG_ERROR(LogEvent, "Failed to find any loader function!");

		if (bWereAllSuccessful)
			*bWereAllSuccessful = false;

		return;
	}

	auto Loader = GetEventLoader();

	if (!Loader)
	{
		if (bWereAllSuccessful)
			*bWereAllSuccessful = false;

		return; // GetEventLoader handles the printing
	}

	Loader->ProcessEvent(LoaderFunction, &OurEvent.AdditionalLoaderParams);
}

static inline bool CallOnReadys(bool* bWereAllSuccessful = nullptr)
{
	if (bWereAllSuccessful)
		*bWereAllSuccessful = false;

	Event OurEvent;

	for (auto& CurrentEvent : Events)
	{
		if (CurrentEvent.Version == Fortnite_Version)
		{
			OurEvent = CurrentEvent;
			break;
		}
	}

	if (!OurEvent.Version)
		return false;

	auto EventScripting = GetEventScripting();

	if (!EventScripting)
		return false; // GetEventScripting handles the printing

	if (bWereAllSuccessful)
		*bWereAllSuccessful = true;

	struct { UObject* GameState; UObject* Playlist; FGameplayTagContainer PlaylistContextTags; } OnReadyParams{ 
		((AFortGameModeAthena*)GetWorld()->GetGameMode())->GetGameStateAthena(), GetEventPlaylist(),
		GetEventPlaylist()->Get<FGameplayTagContainer>("GameplayTagContainer")};

	for (auto& OnReadyFunc : OurEvent.OnReadyFunctions)
	{
		if (OnReadyFunc.first) // func is in loader
		{
			auto EventLoader = GetEventLoader();

			if (!EventLoader)
			{
				// if (bWereAllSuccessful)
					// *bWereAllSuccessful = false;

				continue; // uhh??
			}

			auto OnReadyUFunc = FindObject<UFunction>(OnReadyFunc.second);

			if (!OnReadyUFunc)
			{
				LOG_ERROR(LogEvent, "Failed to find OnReady: {}", OnReadyFunc.second);

				if (bWereAllSuccessful)
					*bWereAllSuccessful = false;

				continue;
			}

			EventLoader->ProcessEvent(OnReadyUFunc, &OnReadyParams);
		}
		else // func is in scripting
		{
			auto OnReadyUFunc = FindObject<UFunction>(OnReadyFunc.second);

			if (!OnReadyUFunc)
			{
				LOG_ERROR(LogEvent, "Failed to find OnReady: {}", OnReadyFunc.second);

				if (bWereAllSuccessful)
					*bWereAllSuccessful = false;

				continue;
			}

			EventScripting->ProcessEvent(OnReadyUFunc, &OnReadyParams);
		}
	}

	return true;
}

static inline void StartEvent()
{
	Event OurEvent;

	for (auto& CurrentEvent : Events)
	{
		if (CurrentEvent.Version == Fortnite_Version)
		{
			OurEvent = CurrentEvent;
			break;
		}
	}

	if (!OurEvent.Version)
		return;

	auto EventScripting = GetEventScripting();

	// if (!EventScripting)
		// return; // GetEventScripting handles the printing

	CallOnReadys();

	for (auto& StartEventFunc : OurEvent.StartEventFunctions)
	{
		auto StartEventUFunc = FindObject<UFunction>(StartEventFunc.first.second);

		if (!StartEventUFunc)
		{
			LOG_ERROR(LogEvent, "Failed to find StartEvent: {}", StartEventFunc.first.second);
			continue;
		}

		if (StartEventFunc.first.first) // func is in loader
		{
			auto EventLoader = GetEventLoader();

			if (!EventLoader)
				continue; // uhh??

			EventLoader->ProcessEvent(StartEventUFunc, &StartEventFunc.second);
		}
		else // func is in scripting
		{
			if (!EventScripting)
				continue;

			EventScripting->ProcessEvent(StartEventUFunc, &StartEventFunc.second);
		}
	}
}

static inline bool DoesEventRequireLoading()
{
	for (auto& CurrentEvent : Events)
	{
		if (CurrentEvent.Version == Fortnite_Version)
		{
			return !CurrentEvent.LoaderClass.empty() && !CurrentEvent.LoaderFunction.empty();
		}
	}

	return false;
}
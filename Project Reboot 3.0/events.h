#pragma once

#include <map>

#include "Object.h"
#include "reboot.h"
#include "GameplayStatics.h"
#include "FortPlaylistAthena.h"

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
	bool RequiredPlaylist = true;
};

static inline std::vector<Event> Events = 
{
	Event
	(
		"Rocket",
		"",
		"",
		0,
		{

		},
		{
			{
				{
					false,
					// "/Buffet/Gameplay/Blueprints/BP_Buffet_Master_Scripting.BP_Buffet_Master_Scripting_C.startevent"
					"/Game/Athena/Maps/Test/Events/BP_GeodeScripting.BP_GeodeScripting_C.LaunchSequence"
				},

				0
			}
		},

		"/Game/Athena/Maps/Test/Events/BP_GeodeScripting.BP_GeodeScripting_C",
		"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo",
		4.5,
		false
	),
	Event
	(
		"Crack Closure",
		"/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C",
		"",
		0,
		{

		},
		{
			{
				{
					false,
					"/Game/Athena/Events/BP_Athena_Event_Components.BP_Athena_Event_Components_C.Final"
				},

				0
			},
			{
				{
					true,
					"/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C.Final"
				},

				0
			},
		},

		"/Game/Athena/Events/BP_Athena_Event_Components.BP_Athena_Event_Components_C",
		"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo",
		5.30,
		false
	),
	/* Event
	(
		"Impact Lake",
		"/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C",
		"",
		0,
		{

		},
		{
			{
				{
					true,
					"/Game/Athena/Prototype/Blueprints/Cube/CUBE.CUBE_C.PlayFinalSink"
				},

				0
			},
		},

		"/Game/Athena/Events/BP_Athena_Event_Components.BP_Athena_Event_Components_C",
		"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo",
		5.41,
		false
	), */
	Event
	(
		"Butterfly",
		"/Game/Athena/Prototype/Blueprints/Island/BP_Butterfly.BP_Butterfly_C",
		"/Game/Athena/Prototype/Blueprints/Island/BP_Butterfly.BP_Butterfly_C.LoadButterflySublevel",
		1,
		{
		},
		{
			{
				{
					true,
					"/Game/Athena/Prototype/Blueprints/Island/BP_Butterfly.BP_Butterfly_C.ButterflySequence" // StartButterflyOnPlaylistLoaded calls this
				},

				0
			}
		},

		"/Game/Athena/Prototype/Blueprints/Island/BP_Butterfly.BP_Butterfly_C",
		"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo",
		6.21,
		false
	),
	Event
	(
		"The End Event Chapter 2",
		"",
		"",
		0,
		{

		},
		{
			{
				{
					false,
					// "/Buffet/Gameplay/Blueprints/BP_Buffet_Master_Scripting.BP_Buffet_Master_Scripting_C.startevent"
					"/Script/SpecialEventGameplayRuntime.SpecialEventScript.StartEventAtIndex"
				},

				0
			}
		},

		"/Guava/Gameplay/BP_Guava_SpecialEventScript.BP_Guava_SpecialEventScript_C", // what
		"/GuavaPlaylist/Playlist/Playlist_Guava.Playlist_Guava",
		18.40
	),
	Event
	(
		"The Showdown",
		"/Game/Athena/Prototype/Blueprints/Cattus/BP_CattusDoggus_Scripting.BP_CattusDoggus_Scripting_C",
		"/Game/Athena/Prototype/Blueprints/Cattus/BP_CattusDoggus_Scripting.BP_CattusDoggus_Scripting_C.LoadCattusLevel", 
		1,
		{
			{
				true,
				"/Game/Athena/Prototype/Blueprints/Cattus/BP_CattusDoggus_Scripting.BP_CattusDoggus_Scripting_C.OnReady_C11CA7624A74FBAEC54753A3C2BD4506"
			}
		},
		{
			{
				{
					true,
					"/Game/Athena/Prototype/Blueprints/Cattus/BP_CattusDoggus_Scripting.BP_CattusDoggus_Scripting_C.startevent"
				},

				0
			}
		},

		"/Game/Athena/Prototype/Blueprints/Cattus/BP_CattusDoggus_Scripting.BP_CattusDoggus_Scripting_C",
		"/Game/Athena/Playlists/Music/Playlist_Music_High.Playlist_Music_High",
		9.40, // also 9.41
		false
	),
	Event
	(
		"The Unvaulting",
		"/Game/Athena/Prototype/Blueprints/White/BP_SnowScripting.BP_SnowScripting_C",
		"/Game/Athena/Prototype/Blueprints/White/BP_SnowScripting.BP_SnowScripting_C.LoadSnowLevel",
		1,
		{
			// todo?
		},
		{
			{
				{
					true,
					"/Game/Athena/Prototype/Blueprints/White/BP_SnowScripting.BP_SnowScripting_C.FinalSequence"
				},

				0
			}
		},

		"/Game/Athena/Prototype/Blueprints/White/BP_SnowScripting.BP_SnowScripting_C",
		"/Game/Athena/Playlists/Music/Playlist_Music_High.Playlist_Music_High",
		8.51
		// Probably requires playlist
	),
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
		"Devourer of Worlds",
		"/Junior/Blueprints/BP_Junior_Loader.BP_Junior_Loader_C",
		"/Junior/Blueprints/BP_Junior_Loader.BP_Junior_Loader_C.LoadJuniorLevel", 
		1,
		{
			{
				false,
				"/Junior/Blueprints/BP_Event_Master_Scripting.BP_Event_Master_Scripting_C.OnReady_872E6C4042121944B78EC9AC2797B053"
			}
		},
		{
			{
				{
					false,
					"/Junior/Blueprints/BP_Junior_Scripting.BP_Junior_Scripting_C.startevent"
				},

				0
			}
		},

		"/Junior/Blueprints/BP_Junior_Scripting.BP_Junior_Scripting_C",
		"/Game/Athena/Playlists/Music/Playlist_Junior_32.Playlist_Junior_32",
		14.60
	),
		Event(
		"The End",
		"",
		"",
		1,
		{
			{
				false,
				"/Game/Athena/Prototype/Blueprints/NightNight/BP_NightNight_Scripting.BP_NightNight_Scripting_C.LoadNightNightLevel" // skunked
			},
			{
				false,
				"/Game/Athena/Prototype/Blueprints/NightNight/BP_NightNight_Scripting.BP_NightNight_Scripting_C.OnReady_D0847F7B4E80F01E77156AA4E7131AF6"
			}
		},
		{
			{
				{
					false,
					"/Game/Athena/Prototype/Blueprints/NightNight/BP_NightNight_Scripting.BP_NightNight_Scripting_C.startevent"
				},

				0
			}
		},

		"/Game/Athena/Prototype/Blueprints/NightNight/BP_NightNight_Scripting.BP_NightNight_Scripting_C",
		"/Game/Athena/Playlists/Music/Playlist_Music_High.Playlist_Music_High",
		10.40,
		false
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
		//"/Game/Athena/Playlists/Fritter/Playlist_Fritter_Lowest.Playlist_Fritter_Lowest",
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
					// "/Game/Athena/Environments/Festivus/Blueprints/BP_FestivusManager.BP_FestivusManager_C.PlayConcert"
					"/Game/Athena/Environments/Festivus/Blueprints/BP_FestivusManager.BP_FestivusManager_C.ServerPlayFestivus" // StartEventFromCalendarAsBackup calls this
				},

				0
			}
		},

		"/Game/Athena/Environments/Festivus/Blueprints/BP_FestivusManager.BP_FestivusManager_C",
		"/Game/Athena/Playlists/Music/Playlist_Music_High.Playlist_Music_High",
		7.30
		// Not sure if this requires playlist.
	),
	Event
	(
		"Rift Tour",
		"",
		"",
		0,
		{

		},
		{
			{
				{
					false,
					// "/Buffet/Gameplay/Blueprints/BP_Buffet_Master_Scripting.BP_Buffet_Master_Scripting_C.startevent"
					"/Script/SpecialEventGameplayRuntime.SpecialEventScript.StartEventAtIndex"
				},

				0
			}
		},

		"/Buffet/Gameplay/Blueprints/Buffet_SpecialEventScript.Buffet_SpecialEventScript_C",
		// "/Buffet/Gameplay/Blueprints/BP_Buffet_Master_Scripting.BP_Buffet_Master_Scripting_C",
		"/BuffetPlaylist/Playlist/Playlist_Buffet.Playlist_Buffet",
		17.30
	),
	Event
    (
        "Operation: Sky Fire",
        "",
        "",
        0,
        {

        },
        {
            {
                {
                    false,
                    // "/Buffet/Gameplay/Blueprints/BP_Buffet_Master_Scripting.BP_Buffet_Master_Scripting_C.startevent"
                    "/Script/SpecialEventGameplayRuntime.SpecialEventScript.StartEventAtIndex"
                },

                0
            }
        },

        "/Kiwi/Gameplay/Kiwi_EventScript.Kiwi_EventScript_C",
        // "/Buffet/Gameplay/Blueprints/BP_Buffet_Master_Scripting.BP_Buffet_Master_Scripting_C",
        "/KiwiPlaylist/Playlists/Playlist_Kiwi.Playlist_Kiwi",
        17.50
    ),
	Event
	(
		"Ice King Event",
		"/Game/Athena/Prototype/Blueprints/Mooney/BP_MooneyLoader.BP_MooneyLoader_C",
		"/Game/Athena/Prototype/Blueprints/Mooney/BP_MooneyLoader.BP_MooneyLoader_C.LoadMap",
		0,
		{
			{
				false,
				"/Game/Athena/Prototype/Blueprints/Mooney/BP_MooneyScripting.BP_MooneyScripting_C.OnReady_9968C1F648044523426FE198948B0CC9"
			}
		},
		{
			{
				{
					false,
					"/Game/Athena/Prototype/Blueprints/Mooney/BP_MooneyScripting.BP_MooneyScripting_C.BeginIceKingEvent"
				},

				0
			}
		},

		"/Game/Athena/Prototype/Blueprints/Mooney/BP_MooneyScripting.BP_MooneyScripting_C",
		"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo",
		7.20
	)	
};

static inline UFortPlaylistAthena* GetEventPlaylist()
{
	for (auto& CurrentEvent : Events)
	{
		if (CurrentEvent.Version == Fortnite_Version)
			return FindObject<UFortPlaylistAthena>(CurrentEvent.PlaylistName, nullptr, ANY_PACKAGE);
	}

	return nullptr;
}

static inline Event GetOurEvent()
{
	for (auto& CurrentEvent : Events)
	{
		if (CurrentEvent.Version == Fortnite_Version)
		{
			return CurrentEvent;
		}
	}

	return Event();
}

static inline bool HasEvent()
{
	return GetOurEvent().Version == Fortnite_Version;
}

static inline bool RequiresEventPlaylist()
{
	return false; // todo
	// return GetOurEvent().PlaylistName != "/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo";
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
		LOG_ERROR(LogEvent, "Failed to find ScriptingClass!");
		return nullptr;
	}

	auto AllScripters = UGameplayStatics::GetAllActorsOfClass(GetWorld(), ScriptingClass);

	if (AllScripters.size() <= 0)
	{
		LOG_ERROR(LogEvent, "Failed to find any scripters!");
		return nullptr;
	}

	return AllScripters.at(0);
}

static inline UObject* GetEventLoader(const std::string& OverrideLoaderName = "NULL")
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

	auto LoaderClass = FindObject<UClass>(OverrideLoaderName == "NULL" ? OurEvent.LoaderClass : OverrideLoaderName);

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

	auto EventPlaylist = GetEventPlaylist();

	struct { UObject* GameState; UObject* Playlist; FGameplayTagContainer PlaylistContextTags; } OnReadyParams{ Cast<AFortGameStateAthena>(GetWorld()->GetGameState()), EventPlaylist };

	if (EventPlaylist)
	{
		static auto GameplayTagContainerOffset = EventPlaylist->GetOffset("GameplayTagContainer");
		OnReadyParams.PlaylistContextTags = EventPlaylist->Get<FGameplayTagContainer>(GameplayTagContainerOffset);
	}
	else
	{
		OnReadyParams.PlaylistContextTags = FGameplayTagContainer();
	}

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

	/* if (Fortnite_Version == 17.30)
	{
		static auto onready = FindObject<UFunction>("/Buffet/Gameplay/Blueprints/BP_Buffet_Master_Scripting.BP_Buffet_Master_Scripting_C.OnReady_C6091CF24046D602CBB778A594DB5BA8");
		auto script = FindObject("/Buffet/Levels/Buffet_P.Buffet_P.PersistentLevel.BP_Event_Master_Scripting_2");

		if (!script)
		{
			LOG_ERROR(LogEvent, "Failed to find MasterScripting");

			if (bWereAllSuccessful)
				*bWereAllSuccessful = false;

			return false;
		}

		script->ProcessEvent(onready, &OnReadyParams);
	} */

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

	LOG_INFO(LogDev, "EventScripting {}", __int64(EventScripting));

	if (EventScripting)
		LOG_INFO(LogDev, "EventScripting Name {}", EventScripting->GetFullName());

	// if (!EventScripting)
		// return; // GetEventScripting handles the printing

	CallOnReadys();

	if (Fortnite_Version >= 17.30)
	{
		static auto OnRep_RootStartTimeFn = FindObject<UFunction>("/Script/SpecialEventGameplayRuntime.SpecialEventScriptMeshActor.OnRep_RootStartTime");
		static auto MeshRootStartEventFn = FindObject<UFunction>("/Script/SpecialEventGameplayRuntime.SpecialEventScriptMeshActor.MeshRootStartEvent");
		auto SpecialEventScriptMeshActorClass = FindObject<UClass>("/Script/SpecialEventGameplayRuntime.SpecialEventScriptMeshActor");
		auto AllSpecialEventScriptMeshActors = UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpecialEventScriptMeshActorClass);

		if (AllSpecialEventScriptMeshActors.Num() > 0)
		{
			auto SpecialEventScriptMeshActor = AllSpecialEventScriptMeshActors.at(0);

			if (SpecialEventScriptMeshActor)
			{
				// if (false)
				{
					LOG_INFO(LogDev, "MeshRootStartEventFn!");
					SpecialEventScriptMeshActor->ProcessEvent(MeshRootStartEventFn);
					SpecialEventScriptMeshActor->ProcessEvent(OnRep_RootStartTimeFn);

					return;
				}
			}
			else
			{
				LOG_ERROR(LogEvent, "Failed to find SpecialEventScriptMeshActor");
			}
		}
		else
		{
			LOG_ERROR(LogEvent, "AllSpecialEventScriptMeshActors.Num() == 0");
		}
	}

	for (auto& StartEventFunc : OurEvent.StartEventFunctions)
	{
		LOG_INFO(LogDev, "Finding {}", StartEventFunc.first.second);

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
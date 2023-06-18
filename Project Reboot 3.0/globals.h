#pragma once

#include <atomic>

#include "inc.h"

namespace Globals
{
	extern inline bool bCreative = false;
	extern inline bool bGoingToPlayEvent = false;
	extern inline bool bEnableAGIDs = true;
	extern inline bool bNoMCP = false;
	extern inline bool bLogProcessEvent = false;
	// extern inline bool bLateGame = false;
	extern inline std::atomic<bool> bLateGame(false);

	extern inline bool bInfiniteMaterials = true;
	extern inline bool bInfiniteAmmo = true;

	extern inline bool bHitReadyToStartMatch = false;
	extern inline bool bInitializedPlaylist = false;
	extern inline bool bStartedListening = false;
	extern inline bool bAutoRestart = false; // doesnt work fyi
	extern inline bool bFillVendingMachines = true;
	extern inline int AmountOfListens = 0; // TODO: Switch to this for LastNum
}

extern inline int NumToSubtractFromSquadId = 0; // I think 2?

extern inline std::string PlaylistName =
"/Game/Athena/Playlists/Playlist_DefaultSquad.Playlist_DefaultSquad";	

// Battle Royale :

// Solos - /Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo
// Duos - /Game/Athena/Playlists/Playlist_DefaultDuo.Playlist_DefaultDuo
// Trios - /Game/Athena/Playlists/Trios/Playlist_Trios.Playlist_Trios
// Squads - /Game/Athena/Playlists/Playlist_DefaultSquad.Playlist_DefaultSquad

// "/Game/Athena/Playlists/gg/Playlist_Gg_Reverse.Playlist_Gg_Reverse";
// "/Game/Athena/Playlists/Playlist_DefaultDuo.Playlist_DefaultDuo";
// "/Game/Athena/Playlists/Playground/Playlist_Playground.Playlist_Playground";
// "/Game/Athena/Playlists/Carmine/Playlist_Carmine.Playlist_Carmine";
// "/Game/Athena/Playlists/Fill/Playlist_Fill_Solo.Playlist_Fill_Solo";
// "/Game/Athena/Playlists/Low/Playlist_Low_Solo.Playlist_Low_Solo";
// "/Game/Athena/Playlists/Bling/Playlist_Bling_Solo.Playlist_Bling_Solo";
// "/Game/Athena/Playlists/Creative/Playlist_PlaygroundV2.Playlist_PlaygroundV2";
// "/Game/Athena/Playlists/Ashton/Playlist_Ashton_Sm.Playlist_Ashton_Sm";
// "/Game/Athena/Playlists/BattleLab/Playlist_BattleLab.Playlist_BattleLab";
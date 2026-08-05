#pragma once

#include <atomic>
#include <string>

#include "inc.h"

namespace Globals
{
    extern inline bool bCreative = false;
    extern inline bool bGoingToPlayEvent = false;
    extern inline bool bEnableAGIDs = false;
    extern inline bool bNoMCP = false;
    extern inline bool bLogProcessEvent = false;
    // extern inline bool bLateGame = false;
    extern inline std::atomic<bool> bLateGame(false);
    extern inline bool bInfiniteMaterials = false;
    extern inline bool bInfiniteAmmo = false;
    extern inline bool bShouldUseReplicationGraph = true;
    extern inline bool bHitReadyToStartMatch = false;
    extern inline bool bInitializedPlaylist = true;
    extern inline bool bStartedListening = false;
    extern inline bool bFillVendingMachines = true;
    extern inline bool bPrivateIPsAreOperator = false;
    extern inline int  AmountOfListens = 0; // TODO: Switch to this for LastNum
    extern inline bool bDeveloperMode = false;

    // ---------------- VBucks / Battlepass / API config ----------------
    extern inline bool bEnableApi = false;
    extern inline std::string ApiKey  = "ur-api-key";
    extern inline std::string WinBaseUrl = "http://127.0.0.1:3551/api/reload/vbucks";
    extern inline std::string KillBaseUrl = "http://127.0.0.1:3551/api/reload/bptiers";

    // ---------------- Auto restart config ----------------
    extern inline bool bAutoRestart = false;
    extern inline float AutoRestartDelaySeconds = 60.0f;
}

extern inline int NumToSubtractFromSquadId = 2;

extern inline std::string PlaylistName = "/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo";

// ---------------- Other Playlists ----------------
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
// "/MoleGame/Playlists/Playlist_MoleGame.Playlist_MoleGame"; // very experimental dont use

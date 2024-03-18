#pragma once

#include <atomic>
#include <mutex>
#include <unordered_map>

#include "inc.h"

namespace Globals
{
	extern inline bool bDisableLootingOnSpawnIsland = false;
	extern inline bool bDisableAutoStart = false; // i dont think i implemented this

	inline std::unordered_map<void*, std::string> playerUsernames; // do not touch it
	inline std::mutex gameThreadLock; // do not touch it

	extern inline int UPTime = 0; // do not touch it
	extern inline int tickTime = 0; // used to detect crashes, do not touch it

	extern inline bool bAllowNoGuiMode = true; // it will start the game server in NO GUI mode if it failes to load the GUI
	
	extern inline bool lateGame = false;
	extern inline bool bRefreshMetsOnKill = true;
	extern inline int refreshMethCount = 150;
	extern inline int StartingShield = /*100*/ 0;

	extern inline bool bSendWebhook = true;
	extern inline std::string EUWebhook = "REAL";
	extern inline std::string NAWebhook = "REAL";
	extern inline std::string Webhook = EUWebhook;
	extern inline std::string EURolePing = "role id";
	extern inline std::string NARolePing = "role id";
	extern inline std::string RolePing = EURolePing;
	extern inline std::string UptimeWebhookMSG = "Server is online, ready up to get into game!";

	extern inline bool shouldKillGame = true;
	extern inline int secBeforeKillGame = 15; //when game ends

	extern inline bool bGetUsernames = false; // do not touch it
	extern inline bool bIsTickTiming = false; // do not touch it
	extern inline bool UPTimeStarted = false; // do not touch it
	extern inline bool bWasSomeoneOnServerBefore = false; // do not touch it
	extern inline bool bSentStart = false; // do not touch it
	extern inline bool bSentEnd = false; // do not touch it
	extern inline bool wasWebhookSent = false; // do not touch it
	extern inline bool IsGuiAlive = false; // do not touch it

	extern inline bool bAllowRebootPlayer = true;
	extern inline bool bRemoveRebootFromPlayerName = true; // buggy

	extern inline int RequiredPlayers = 2;
	extern inline int BusLaunchTime = 50;
	extern inline bool bEnoughPlayers = false; // do not touch it
	extern inline bool bStarting = false; // do not touch it
	extern inline bool bStarted = false; // do not touch it
	extern inline bool bEnded = false; // do not touch it
	extern inline bool bCreative = false;
	extern inline bool bGoingToPlayEvent = false;
	extern inline bool bEnableAGIDs = true;
	extern inline bool bNoMCP = false;
	extern inline bool bLogProcessEvent = false;
	// extern inline bool bLateGame = false;
	extern inline std::atomic<bool> bLateGame(Globals::lateGame); // do not touch it

	extern inline bool bInfiniteMaterials = false;
	extern inline bool bInfiniteAmmo = false;

	extern inline bool bHitReadyToStartMatch = false;
	extern inline bool bInitializedPlaylist = false;
	extern inline bool bStartedListening = false;
	extern inline bool bAutoRestart = true; // doesnt work fyi
	extern inline bool bFillVendingMachines = true;
	extern inline int AmountOfListens = 0;  // TODO: Switch to this for LastNum
	extern inline std::string GamemodeToSay = ""; //Your playlist, has to be in this format (bump workflow)
	extern inline std::string PlaylistName = "/KiwiPlaylist/Playlists/Playlist_Kiwi.Playlist_Kiwi";
	extern inline std::string Bote = "Flipped"; //Your backend name (Has to be the same one as in .env)
	extern inline std::string LoopKey = "your loop key"; //Your LINK api key
	extern inline void* BM = nullptr;
}

extern inline int NumToSubtractFromSquadId = 0; // I think 2?

extern inline std::string PlaylistName =
"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo";
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

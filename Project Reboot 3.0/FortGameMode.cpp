#include "FortGameMode.h"

void AFortGameMode::SetCurrentPlaylistName(UFortPlaylist* Playlist) // Techinally it takes in a fname
{
	if (!Playlist)
	{
		LOG_WARN(LogGame, "AFortGameMode::SetCurrentPlaylistName: Invalid playlist.");
		return;
	}

	static auto PlaylistNameOffset = Playlist->GetOffset("PlaylistName");

	static auto CurrentPlaylistNameOffset = GetOffset("CurrentPlaylistName");
	static auto CurrentPlaylistIdOffset = GetOffset("CurrentPlaylistId");

	Get<FName>(CurrentPlaylistNameOffset) = Playlist->Get<FName>(PlaylistNameOffset);
	Get<int>(CurrentPlaylistIdOffset) = Playlist->GetPlaylistId();
}
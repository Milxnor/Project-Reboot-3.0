#pragma once

#include "GameMode.h"
#include "FortPlaylist.h"

class AFortGameMode : public AGameMode
{
public:
	void SetCurrentPlaylistName(UFortPlaylist* Playlist); // Techinally it takes in a fname
};
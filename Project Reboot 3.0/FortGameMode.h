#pragma once

#include "GameMode.h"

class AFortGameMode : public AGameMode
{
public:
	void SetCurrentPlaylistName(UObject* Playlist); // Techinally it takes in a fname
};
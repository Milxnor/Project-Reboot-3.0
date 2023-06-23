#pragma once

#include "FortPlaylist.h"

enum class EAthenaRespawnType : uint8_t
{
	None = 0,
	InfiniteRespawn = 1,
	InfiniteRespawnExceptStorm = 2,
	EAthenaRespawnType_MAX = 3
};

class UFortPlaylistAthena : public UFortPlaylist
{
public:
	EAthenaRespawnType& GetRespawnType()
	{
		static auto RespawnTypeOffset = GetOffset("RespawnType");
		return Get<EAthenaRespawnType>(RespawnTypeOffset);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortPlaylistAthena");
		return Class;
	}
};
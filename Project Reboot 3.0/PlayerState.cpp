#include "PlayerState.h"

#include "reboot.h"

int& APlayerState::GetPlayerID()
{
	static auto PlayerIDOffset = FindOffsetStruct("/Script/Engine.PlayerState", "PlayerID", false);

	if (PlayerIDOffset == -1)
	{
		static auto PlayerIdOffset = FindOffsetStruct("/Script/Engine.PlayerState", "PlayerId", false);
		return Get<int>(PlayerIdOffset);
	}

	return Get<int>(PlayerIDOffset);
}
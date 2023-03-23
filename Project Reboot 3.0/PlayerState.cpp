#include "PlayerState.h"

#include "reboot.h"

int& APlayerState::GetPlayerID()
{
	static auto PlayerIDOffset = FindOffsetStruct("/Script/Engine.PlayerState", "PlayerID");
	return Get<int>(PlayerIDOffset);
}
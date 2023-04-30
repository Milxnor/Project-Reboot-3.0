#include "PlayerState.h"

#include "reboot.h"

FString APlayerState::GetPlayerName()
{
	static auto GetPlayerNameFn = FindObject<UFunction>("/Script/Engine.PlayerState.GetPlayerName");

	if (GetPlayerNameFn)
	{
		FString PlayerName;
		this->ProcessEvent(GetPlayerNameFn, &PlayerName);
		return PlayerName;
	}

	static auto PlayerNameOffset = GetOffset("PlayerName");
	return Get<FString>(PlayerNameOffset);
}

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
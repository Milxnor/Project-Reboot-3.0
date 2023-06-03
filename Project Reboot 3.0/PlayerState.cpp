#include "PlayerState.h"

#include "reboot.h"

FString& APlayerState::GetSavedNetworkAddress()
{
	static auto SavedNetworkAddressOffset = GetOffset("SavedNetworkAddress");
	return Get<FString>(SavedNetworkAddressOffset);
}

FString APlayerState::GetPlayerName()
{
	static auto GetPlayerNameFn = FindObject<UFunction>(L"/Script/Engine.PlayerState.GetPlayerName");

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

bool APlayerState::IsBot()
{
	static auto bIsABotOffset = GetOffset("bIsABot");
	static auto bIsABotFieldMask = GetFieldMask(GetProperty("bIsABot"));
	return ReadBitfieldValue(bIsABotOffset, bIsABotFieldMask);
}

void APlayerState::SetIsBot(bool NewValue)
{
	static auto bIsABotOffset = GetOffset("bIsABot");
	static auto bIsABotFieldMask = GetFieldMask(GetProperty("bIsABot"));
	return SetBitfieldValue(bIsABotOffset, bIsABotFieldMask, NewValue);
}

void APlayerState::OnRep_PlayerName()
{
	static auto OnRep_PlayerNameFn = FindObject<UFunction>("/Script/Engine.PlayerState.OnRep_PlayerName");
	this->ProcessEvent(OnRep_PlayerNameFn);
}
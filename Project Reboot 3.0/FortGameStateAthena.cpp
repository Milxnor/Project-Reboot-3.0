#include "FortGameStateAthena.h"

#include "reboot.h"
#include "FortPlayerStateAthena.h"

/* void AFortGameStateAthena::AddPlayerStateToGameMemberInfo(class AFortPlayerStateAthena* PlayerState)
{

} */

UObject*& AFortGameStateAthena::GetCurrentPlaylist()
{
	static auto CurrentPlaylistInfoOffset = GetOffset("CurrentPlaylistInfo", false);

	if (CurrentPlaylistInfoOffset == 0)
	{
		static auto CurrentPlaylistDataOffset = GetOffset("CurrentPlaylistData");
		return Get(CurrentPlaylistDataOffset);
	}

	auto CurrentPlaylistInfo = this->GetPtr<FFastArraySerializer>(CurrentPlaylistInfoOffset);

	static auto BasePlaylistOffset = FindOffsetStruct("/Script/FortniteGame.PlaylistPropertyArray", "BasePlaylist");
	return *(UObject**)(__int64(CurrentPlaylistInfo) + BasePlaylistOffset);
}

int AFortGameStateAthena::GetAircraftIndex(AFortPlayerState* PlayerState)
{
	// The function has a string in it but we can just remake lol

	auto PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerState);

	if (!PlayerStateAthena)
		return 0;

	auto CurrentPlaylist = GetCurrentPlaylist();
	
	if (!CurrentPlaylist)
		return 0;

	static auto AirCraftBehaviorOffset = GetOffset("AirCraftBehavior");

	if (Get<uint8_t>(AirCraftBehaviorOffset) != 1) // AirCraftBehavior != EAirCraftBehavior::OpposingAirCraftForEachTeam
		return 0;

	auto TeamIndex = PlayerStateAthena->GetTeamIndex();
	int idfkwhatthisisimguessing = TeamIndex;

	static auto DefaultFirstTeamOffset = CurrentPlaylist->GetOffset("DefaultFirstTeam");
	auto DefaultFirstTeam = CurrentPlaylist->Get<int>(DefaultFirstTeamOffset);

	return TeamIndex - idfkwhatthisisimguessing;
}

bool AFortGameStateAthena::IsRespawningAllowed(AFortPlayerState* PlayerState) // actually in zone
{
	static auto IsRespawningAllowedFn = FindObject<UFunction>("/Script/FortniteGame.FortGameStateZone.IsRespawningAllowed");

	if (!IsRespawningAllowedFn)
		return false;

	struct { AFortPlayerState* PlayerState; bool ReturnValue; } AFortGameStateZone_IsRespawningAllowed_Params{PlayerState};
	this->ProcessEvent(IsRespawningAllowedFn, &AFortGameStateZone_IsRespawningAllowed_Params);

	return AFortGameStateZone_IsRespawningAllowed_Params.ReturnValue;
}

void AFortGameStateAthena::OnRep_GamePhase()
{
	EAthenaGamePhase OldGamePhase = GetGamePhase();

	static auto OnRep_GamePhase = FindObject<UFunction>("/Script/FortniteGame.FortGameStateAthena.OnRep_GamePhase");
	this->ProcessEvent(OnRep_GamePhase, &OldGamePhase);
}

void AFortGameStateAthena::OnRep_CurrentPlaylistInfo()
{
	static auto OnRep_CurrentPlaylistData = FindObject<UFunction>("/Script/FortniteGame.FortGameStateAthena.OnRep_CurrentPlaylistData");

	if (OnRep_CurrentPlaylistData)
	{
		this->ProcessEvent(OnRep_CurrentPlaylistData);
	}
	else
	{
		static auto OnRep_CurrentPlaylistInfo = FindObject<UFunction>("/Script/FortniteGame.FortGameStateAthena.OnRep_CurrentPlaylistInfo");

		if (OnRep_CurrentPlaylistInfo)
			this->ProcessEvent(OnRep_CurrentPlaylistInfo);
	}
}
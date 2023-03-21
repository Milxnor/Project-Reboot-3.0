#include "FortGameStateAthena.h"

#include "reboot.h"

/* void AFortGameStateAthena::AddPlayerStateToGameMemberInfo(class AFortPlayerStateAthena* PlayerState)
{

} */

bool AFortGameStateAthena::IsRespawningAllowed(AFortPlayerState* PlayerState) // actually in zone
{
	static auto IsRespawningAllowedFn = FindObject<UFunction>("/Script/FortniteGame.FortGameStateZone.IsRespawningAllowed");
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
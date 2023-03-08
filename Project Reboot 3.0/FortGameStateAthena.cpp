#include "FortGameStateAthena.h"

#include "reboot.h"

/* void AFortGameStateAthena::AddPlayerStateToGameMemberInfo(class AFortPlayerStateAthena* PlayerState)
{

} */

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
		this->ProcessEvent(OnRep_CurrentPlaylistInfo);
	}
}
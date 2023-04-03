#include "FortGameStateAthena.h"

#include "reboot.h"
#include "FortPlayerStateAthena.h"
#include "FortGameModeAthena.h"

/* void AFortGameStateAthena::AddPlayerStateToGameMemberInfo(class AFortPlayerStateAthena* PlayerState)
{

} */

UObject*& AFortGameStateAthena::GetCurrentPlaylist()
{
	static auto CurrentPlaylistInfoOffset = GetOffset("CurrentPlaylistInfo", false);

	if (CurrentPlaylistInfoOffset == -1)
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
	auto GameModeAthena = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
	static auto IsRespawningAllowedFn = FindObject<UFunction>("/Script/FortniteGame.FortGameStateZone.IsRespawningAllowed");

	LOG_INFO(LogDev, "IsRespawningAllowedFn: {}", __int64(IsRespawningAllowedFn));

	if (!IsRespawningAllowedFn)
	{
		static auto CurrentPlaylistDataOffset = GetOffset("CurrentPlaylistData", false);
		auto CurrentPlaylist = CurrentPlaylistDataOffset == -1 && Fortnite_Version < 6 ? nullptr : GetCurrentPlaylist();

		if (!CurrentPlaylist)
			return false;

		static auto RespawnTypeOffset = CurrentPlaylist->GetOffset("RespawnType");

		if (RespawnTypeOffset == -1)
			return false;

		auto& RespawnType = CurrentPlaylist->Get<uint8_t>(RespawnTypeOffset);
		LOG_INFO(LogDev, "RespawnType: {}", (int)RespawnType);

		if (RespawnType == 1)
			return true;
		
		if (RespawnType == 2) // InfiniteRespawnExceptStorm
		{
			static auto SafeZoneIndicatorOffset = GameModeAthena->GetOffset("SafeZoneIndicator");
			auto SafeZoneIndicator = GameModeAthena->Get<AActor*>(SafeZoneIndicatorOffset);

			if (!SafeZoneIndicator)
				return true;

			/*
			
			10.40

			bool __fastcall sub_7FF68F5A83A0(__int64 SafeZoneIndicator, float *a2)
			{
			  __m128 v2; // xmm1
			  float v3; // xmm2_4

			  v2 = *(*(SafeZoneIndicator + 928) + 464i64);
			  v3 = _mm_shuffle_ps(v2, v2, 85).m128_f32[0];
			  return (*(SafeZoneIndicator + 924) * *(SafeZoneIndicator + 924)) >= (((v3 - a2[1]) * (v3 - a2[1]))
																				 + ((v2.m128_f32[0] - *a2) * (v2.m128_f32[0] - *a2)));
			}

			If this returns true, then return true

			*/

			return true; // Do this until we implement ^^
		}

		return false;
	}

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
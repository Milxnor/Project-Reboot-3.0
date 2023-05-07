#include "FortGameStateAthena.h"

#include "reboot.h"
#include "FortPlayerStateAthena.h"
#include "FortGameModeAthena.h"
#include "FortAthenaMutator.h"

/* void AFortGameStateAthena::AddPlayerStateToGameMemberInfo(class AFortPlayerStateAthena* PlayerState)
{

} */

TScriptInterface<UFortSafeZoneInterface> AFortGameStateAthena::GetSafeZoneInterface()
{
	int Offset = -1;

	if (Fortnite_Version == 10.40)
	{
		// Offset = 0xF60;
	}

	TScriptInterface<UFortSafeZoneInterface> ScriptInterface{};

	if (Offset != -1)
	{
		auto idk = (void*)(__int64(this) + Offset);

		UObject* ObjectPtr = reinterpret_cast<UObject* (*)(__int64)>(((UObject*)idk)->VFTable[0x1])(__int64(idk)); // not actually a uobject but its just how we can get vft

		if (ObjectPtr)
		{
			ScriptInterface.ObjectPointer = ObjectPtr;
			ScriptInterface.InterfacePointer = ObjectPtr->GetInterfaceAddress(UFortSafeZoneInterface::StaticClass());
		}
	}

	return ScriptInterface;
}

void AFortGameStateAthena::SetGamePhaseStep(EAthenaGamePhaseStep NewGamePhaseStep)
{
	this->GetGamePhaseStep() = NewGamePhaseStep;

	std::vector<std::pair<AFortAthenaMutator*, UFunction*>> FunctionsToCall;

	LoopMutators([&](AFortAthenaMutator* Mutator) { FunctionsToCall.push_back(std::make_pair(Mutator, Mutator->FindFunction("OnGamePhaseStepChanged"))); });

	for (auto& FunctionToCallPair : FunctionsToCall)
	{
		// On newer versions there is a second param.

		LOG_INFO(LogDev, "A1: {} FunctionToCallPair.second: {}", FunctionToCallPair.first->IsValidLowLevel() ? FunctionToCallPair.first->GetFullName() : "BadRead", __int64(FunctionToCallPair.second));

		if (FunctionToCallPair.second->IsValidLowLevel() && FunctionToCallPair.first->IsValidLowLevel())
		{
			auto Params = ConstructOnGamePhaseStepChangedParams(NewGamePhaseStep);

			if (Params)
			{
				FunctionToCallPair.first->ProcessEvent(FunctionToCallPair.second, Params);
				VirtualFree(Params, 0, MEM_RELEASE);
			}
		}
	}
}

UFortPlaylist*& AFortGameStateAthena::GetCurrentPlaylist()
{
	static auto CurrentPlaylistInfoOffset = GetOffset("CurrentPlaylistInfo", false);

	if (CurrentPlaylistInfoOffset == -1)
	{
		static auto CurrentPlaylistDataOffset = GetOffset("CurrentPlaylistData");
		return Get<UFortPlaylist*>(CurrentPlaylistDataOffset);
	}

	auto CurrentPlaylistInfo = this->GetPtr<FFastArraySerializer>(CurrentPlaylistInfoOffset);

	static auto BasePlaylistOffset = FindOffsetStruct("/Script/FortniteGame.PlaylistPropertyArray", "BasePlaylist");
	return *(UFortPlaylist**)(__int64(CurrentPlaylistInfo) + BasePlaylistOffset);
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

bool AFortGameStateAthena::IsPlayerBuildableClass(UClass* Class)
{
	return true;

	static auto AllPlayerBuildableClassesOffset = GetOffset("AllPlayerBuildableClasses", false);

	if (AllPlayerBuildableClassesOffset == -1) // this is invalid in like s6 and stuff we need to find a better way to do this
		return true;

	auto& AllPlayerBuildableClasses = Get<TArray<UClass*>>(AllPlayerBuildableClassesOffset);

	for (int j = 0; j < AllPlayerBuildableClasses.Num(); j++)
	{
		auto CurrentPlayerBuildableClass = AllPlayerBuildableClasses.at(j);

		// LOG_INFO(LogDev, "CurrentPlayerBuildableClass: {}", CurrentPlayerBuildableClass->GetFullName());

		if (CurrentPlayerBuildableClass == Class)
			return true;
	}

	return false;

	// I don't know why but I think these are empty

	auto PlayerBuildableClasses = GetPlayerBuildableClasses();

	int ArraySize = 4 - 1;

	for (int i = 0; i < ArraySize; i++)
	{
		auto CurrentPlayerBuildableClassesArray = PlayerBuildableClasses[i].BuildingClasses;

		for (int j = 0; j < CurrentPlayerBuildableClassesArray.Num(); j++)
		{
			auto CurrentPlayerBuildableClass = CurrentPlayerBuildableClassesArray.at(j);

			LOG_INFO(LogDev, "CurrentPlayerBuildableClass: {}", CurrentPlayerBuildableClass->GetFullName());

			if (CurrentPlayerBuildableClass == Class)
				return true;
		}
	}

	return false;
}

bool AFortGameStateAthena::IsRespawningAllowed(AFortPlayerState* PlayerState)
{
	auto GameModeAthena = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
	static auto IsRespawningAllowedFn = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateZone.IsRespawningAllowed");

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

	static auto OnRep_GamePhase = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateAthena.OnRep_GamePhase");
	this->ProcessEvent(OnRep_GamePhase, &OldGamePhase);
}

void AFortGameStateAthena::OnRep_CurrentPlaylistInfo()
{
	static auto OnRep_CurrentPlaylistData = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateAthena.OnRep_CurrentPlaylistData");

	if (OnRep_CurrentPlaylistData)
	{
		this->ProcessEvent(OnRep_CurrentPlaylistData);
	}
	else
	{
		static auto OnRep_CurrentPlaylistInfo = FindObject<UFunction>(L"/Script/FortniteGame.FortGameStateAthena.OnRep_CurrentPlaylistInfo");

		if (OnRep_CurrentPlaylistInfo)
			this->ProcessEvent(OnRep_CurrentPlaylistInfo);
	}
}

void AFortGameStateAthena::OnRep_PlayersLeft()
{
	static auto OnRep_PlayersLeftFn = FindObject<UFunction>("/Script/FortniteGame.FortGameStateAthena.OnRep_PlayersLeft");

	if (!OnRep_PlayersLeftFn)
		return;

	this->ProcessEvent(OnRep_PlayersLeftFn);
}
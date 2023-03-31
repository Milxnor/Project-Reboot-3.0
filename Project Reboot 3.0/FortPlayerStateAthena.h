#pragma once

#include "FortPlayerState.h"
#include "Stack.h"

class AFortPlayerStateAthena : public AFortPlayerState
{
public:
	static inline void (*ServerSetInAircraftOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	
	uint8& GetSquadId()
	{
		static auto SquadIdOffset = GetOffset("SquadId");
		return Get<uint8>(SquadIdOffset);
	}

	uint8& GetTeamIndex()
	{
		static auto TeamIndexOffset = GetOffset("TeamIndex");
		return Get<uint8>(TeamIndexOffset);
	}

	FString GetPlayerName()
	{
		static auto GetPlayerNameFn = FindObject<UFunction>("/Script/Engine.PlayerState.GetPlayerName");
		FString PlayerName;
		this->ProcessEvent(GetPlayerNameFn, &PlayerName);
		return PlayerName;
	}

	void ClientReportKill(AFortPlayerStateAthena* Player)
	{
		static auto ClientReportKillFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerStateAthena.ClientReportKill");
		this->ProcessEvent(ClientReportKillFn, &Player);
	}

	static void ServerSetInAircraftHook(UObject* Context, FFrame& Stack, void* Ret);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortPlayerStateAthena");
		return Class;
	}
};
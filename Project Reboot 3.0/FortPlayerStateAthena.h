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

	int& GetPlace()
	{
		static auto PlaceOffset = GetOffset("Place");
		return Get<int>(PlaceOffset);
	}

	bool IsInAircraft()
	{
		static auto bInAircraftOffset = GetOffset("bInAircraft");
		static auto bInAircraftFieldMask = GetFieldMask(GetProperty("bInAircraft"));
		return ReadBitfieldValue(bInAircraftOffset, bInAircraftFieldMask);
	}

	bool HasThankedBusDriver()
	{
		static auto bThankedBusDriverOffset = GetOffset("bThankedBusDriver");
		static auto bThankedBusDriverFieldMask = GetFieldMask(GetProperty("bThankedBusDriver"));
		return ReadBitfieldValue(bThankedBusDriverOffset, bThankedBusDriverFieldMask);
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
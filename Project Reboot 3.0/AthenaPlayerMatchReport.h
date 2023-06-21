#pragma once

#include "reboot.h"

struct FAthenaMatchTeamStats
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.AthenaMatchTeamStats");
		return Struct;
	}

	static auto GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	int                                                Place;                                                    // 0x0000(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                TotalPlayers;                                             // 0x0004(0x0004) (Edit, BlueprintVisible, BlueprintReadOnly, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	int& GetPlace()
	{
		return Place;
	}

	int& GetTotalPlayers()
	{
		return TotalPlayers;
	}
};

class UAthenaPlayerMatchReport : public UObject
{
public:
	bool& HasTeamStats()
	{
		static auto bHasTeamStatsOffset = GetOffset("bHasTeamStats");
		return Get<bool>(bHasTeamStatsOffset);
	}

	bool& HasMatchStats()
	{
		static auto bHasMatchStatsOffset = GetOffset("bHasMatchStats");
		return Get<bool>(bHasMatchStatsOffset);
	}

	FAthenaMatchTeamStats* GetTeamStats()
	{
		static auto TeamStatsOffset = GetOffset("TeamStats");
		return GetPtr<FAthenaMatchTeamStats>(TeamStatsOffset);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.AthenaPlayerMatchReport");
		return Class;
	}
};
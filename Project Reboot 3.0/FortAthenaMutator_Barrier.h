// Food Fight

#pragma once

#include "FortAthenaMutator.h"
#include "AthenaBigBaseWall.h"
#include "AthenaBarrierObjective.h"
#include "AthenaBarrierFlag.h"

/* 

EVENT IDS (got on 10.40):

WallComingDown - 1
WallDown - 2
// IDK REST COMPILER WAS TOO SMART
Intro - 9
NoMoreRespawns - 10

*/

struct FBarrierTeamState // Idk if this actually changes
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.BarrierTeamState");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	EBarrierFoodTeam& GetFoodTeam()
	{
		static auto FoodTeamOffset = FindOffsetStruct("/Script/FortniteGame.BarrierTeamState", "FoodTeam");
		return *(EBarrierFoodTeam*)(__int64(this) + FoodTeamOffset);
	}

	AAthenaBarrierFlag*& GetObjectiveFlag()
	{
		static auto ObjectiveFlagOffset = FindOffsetStruct("/Script/FortniteGame.BarrierTeamState", "ObjectiveFlag");
		return *(AAthenaBarrierFlag**)(__int64(this) + ObjectiveFlagOffset);
	}

	AAthenaBarrierObjective*& GetObjectiveObject()
	{
		static auto ObjectiveObjectOffset = FindOffsetStruct("/Script/FortniteGame.BarrierTeamState", "ObjectiveObject");
		return *(AAthenaBarrierObjective**)(__int64(this) + ObjectiveObjectOffset);
	}
};

class AFortAthenaMutator_Barrier : public AFortAthenaMutator
{
public:
	static inline void (*OnGamePhaseStepChangedOriginal)(UObject* Context, FFrame& Stack, void* Ret);

	UClass* GetBigBaseWallClass()
	{
		static auto BigBaseWallClassOffset = GetOffset("BigBaseWallClass");
		return Get<UClass*>(BigBaseWallClassOffset);
	}

	UClass* GetObjectiveFlagClass()
	{
		static auto ObjectiveFlagOffset = GetOffset("ObjectiveFlag");
		return Get<UClass*>(ObjectiveFlagOffset);
	}

	AAthenaBigBaseWall*& GetBigBaseWall()
	{
		static auto BigBaseWallOffset = GetOffset("BigBaseWall");
		return Get<AAthenaBigBaseWall*>(BigBaseWallOffset);
	}

	FBarrierTeamState* GetTeam_0_State()
	{
		static auto Team_0_StateOffset = GetOffset("Team_0_State");
		return GetPtr<FBarrierTeamState>(Team_0_StateOffset);
	}

	FBarrierTeamState* GetTeam_1_State()
	{
		static auto Team_1_StateOffset = GetOffset("Team_1_State");
		return GetPtr<FBarrierTeamState>(Team_1_StateOffset);
	}

	static void OnGamePhaseStepChangedHook(UObject* Context, FFrame& Stack, void* Ret);
};
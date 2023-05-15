#pragma once

#include "ActorComponent.h"

#include "FortAthenaPatrolPath.h"

#include "reboot.h"

class UFortAthenaNpcPatrollingComponent : public UActorComponent
{
public:
	void SetPatrolPath(AFortAthenaPatrolPath* NewPatrolPath)
	{
		static auto SetPatrolPathFn = FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaNpcPatrollingComponent:SetPatrolPath");
		this->ProcessEvent(SetPatrolPathFn, &NewPatrolPath);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaNpcPatrollingComponent");
		return Class;
	}
};
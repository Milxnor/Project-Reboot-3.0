#pragma once

#include "BuildingSMActor.h"
#include "FortWeapon.h"

class AFortWeap_BuildingToolBase : public AFortWeapon
{
};

class AFortWeap_BuildingTool : public AFortWeap_BuildingToolBase
{
};

class AFortWeap_EditingTool : public AFortWeap_BuildingToolBase
{
public:
	static inline void (*originalOnRep_EditActor)(AFortWeap_EditingTool*);

	ABuildingSMActor*& GetEditActor()
	{
		static auto EditActorOffset = GetOffset("EditActor");
		return Get<ABuildingSMActor*>(EditActorOffset);
	}

	void OnRep_EditActor();

	void SetEditActor(ABuildingSMActor* EditActor)
	{
		// if (HasAuthority())
		{
			GetEditActor() = EditActor;
			originalOnRep_EditActor(this);
		}
	}

	static UClass* StaticClass();
};
#pragma once

#include "BuildingGameplayActor.h"
#include "AthenaBarrierObjective.h"

struct FBarrierFlagDisplayData
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.BarrierFlagDisplayData");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	UStaticMesh* GetHeadMesh()
	{
		static auto HeadMeshOffset = FindOffsetStruct("/Script/FortniteGame.BarrierFlagDisplayData", "HeadMesh");
		return *(UStaticMesh**)(__int64(this) + HeadMeshOffset);
	}

	FVector& GetMeshScale()
	{
		static auto MeshScaleOffset = FindOffsetStruct("/Script/FortniteGame.BarrierFlagDisplayData", "MeshScale");
		return *(FVector*)(__int64(this) + MeshScaleOffset);
	}
};

class AAthenaBarrierFlag : public ABuildingGameplayActor
{
public:
	EBarrierFoodTeam& GetFoodTeam()
	{
		static auto FoodTeamOffset = GetOffset("FoodTeam");
		return Get<EBarrierFoodTeam>(FoodTeamOffset);
	}

	FBarrierFlagDisplayData* GetDisplayData(EBarrierFoodTeam FoodTeam)
	{
		static auto FoodDisplayDataOffset = GetOffset("FoodDisplayData");
		auto FoodDisplayData = Get<FBarrierFlagDisplayData*>(FoodDisplayDataOffset); // Array of size 2
		return &FoodDisplayData[(int)FoodTeam];
	}
};
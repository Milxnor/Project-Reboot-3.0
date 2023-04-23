#pragma once

#include "BuildingGameplayActor.h"

using UStaticMesh = UObject;
using UMaterialInterface = UObject;

enum class EBarrierFoodTeam : uint8_t
{
	Burger = 0,
	Tomato = 1,
	MAX = 2
};

struct FBarrierObjectiveDisplayData
{
	UStaticMesh* HeadMesh;                                                 // 0x0000(0x0008) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     MeshScale;                                                // 0x0008(0x000C) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     MeshRelativeOffset;                                       // 0x0014(0x000C) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	TArray<UMaterialInterface*>                  MaterialsToSwap;                                          // 0x0020(0x0010) (Edit, ZeroConstructor, NativeAccessSpecifierPublic)
};

class AAthenaBarrierObjective : public ABuildingGameplayActor
{
public:
	void SetHeadMesh(UStaticMesh* NewMesh, const FVector& NewScale, const FVector& NewOffset, const TArray<UMaterialInterface*>& MaterialsToSwap)
	{
		static auto SetHeadMeshFn = FindObject<UFunction>("/Script/FortniteGame.AthenaBarrierObjective.SetHeadMesh");

		struct
		{
			UStaticMesh* NewMesh;                                                  // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FVector                                     NewScale;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FVector                                     NewOffset;                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			TArray<UMaterialInterface*>                  MaterialsToSwap;                                          // (ConstParm, Parm, OutParm, ZeroConstructor, ReferenceParm, NativeAccessSpecifierPublic)
		} AAthenaBarrierObjective_SetHeadMesh_Params{ NewMesh, NewScale, NewOffset, MaterialsToSwap };

		this->ProcessEvent(SetHeadMeshFn, &AAthenaBarrierObjective_SetHeadMesh_Params);
	}

	EBarrierFoodTeam& GetFoodTeam()
	{
		static auto FoodTeamOffset = GetOffset("FoodTeam");
		return Get<EBarrierFoodTeam>(FoodTeamOffset);
	}

	FBarrierObjectiveDisplayData* GetFoodDisplayData(EBarrierFoodTeam FoodTeam)
	{
		static auto FoodDisplayDataOffset = GetOffset("FoodDisplayData");
		auto FoodDisplayData = Get<FBarrierObjectiveDisplayData*>(FoodDisplayDataOffset); // Array size of 2
		return &FoodDisplayData[(int)FoodTeam];
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.AthenaBarrierObjective");
		return Class;
	}
};
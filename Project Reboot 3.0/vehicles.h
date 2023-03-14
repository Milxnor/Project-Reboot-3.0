#pragma once

#include "reboot.h"
#include "Stack.h"
#include "Actor.h"
#include "hooking.h"
#include "SoftObjectPtr.h"

// Vehicle class name changes multiple times across versions, so I made it it's own file.

static inline void ServerVehicleUpdate(UObject* Context, FFrame& Stack, void* Ret)
{
	auto Params = Stack.Locals;

	auto Vehicle = Cast<AActor>(Context);
	static auto RootComponentOffset = Vehicle->GetOffset("RootComponent");
	auto Mesh = /* Cast<UPrimitiveComponent> */(Vehicle->Get(RootComponentOffset));

	FTransform Transform{};

	static std::string StateStructName = FindObject("/Script/FortniteGame.ReplicatedPhysicsPawnState") ? "/Script/FortniteGame.ReplicatedPhysicsPawnState" : "";

	if (StateStructName.empty())
		return;

	auto State = (void*)(__int64(Params) + 0);

	static auto RotationOffset = FindOffsetStruct(StateStructName, "Rotation");
	static auto TranslationOffset = FindOffsetStruct(StateStructName, "Translation");

	if (std::floor(Engine_Version) >= 423)
	{
		float v50 = -2.0;
		float v49 = 2.5;

		auto Rotation = (FQuat*)(__int64(State) + RotationOffset);

		Rotation->X -= v49;
		Rotation->Y /= 0.3;
		Rotation->Z -= v50;
		Rotation->W /= -1.2;

		Transform.Rotation = *Rotation;
	}

	Transform.Translation = *(FVector*)(__int64(State) + TranslationOffset);
	Transform.Scale3D = FVector{ 1, 1, 1 };

	bool bTeleport = true; // this maybe be false??
	bool bSweep = false;

	static auto K2_SetWorldTransformFn = FindObject<UFunction>(L"/Script/Engine.SceneComponent.K2_SetWorldTransform");
	static auto SetPhysicsLinearVelocityFn = FindObject<UFunction>(L"/Script/Engine.PrimitiveComponent.SetPhysicsLinearVelocity");
	static auto SetPhysicsAngularVelocityFn = FindObject<UFunction>(L"/Script/Engine.PrimitiveComponent.SetPhysicsAngularVelocity");
	static auto LinearVelocityOffset = FindOffsetStruct(StateStructName, "LinearVelocity");
	static auto AngularVelocityOffset = FindOffsetStruct(StateStructName, "AngularVelocity");
	static auto K2_SetWorldTransformParamSize = K2_SetWorldTransformFn->GetPropertiesSize();

	auto K2_SetWorldTransformParams = Alloc(K2_SetWorldTransformParamSize);

	{
		static auto NewTransformOffset = FindOffsetStruct("/Script/Engine.SceneComponent.K2_SetWorldTransform", "NewTransform");
		static auto bSweepOffset = FindOffsetStruct("/Script/Engine.SceneComponent.K2_SetWorldTransform", "bSweep");
		static auto bTeleportOffset = FindOffsetStruct("/Script/Engine.SceneComponent.K2_SetWorldTransform", "bTeleport");

		*(FTransform*)(__int64(K2_SetWorldTransformParams) + NewTransformOffset) = Transform;
		*(bool*)(__int64(K2_SetWorldTransformParams) + bSweepOffset) = bSweep;
		*(bool*)(__int64(K2_SetWorldTransformParams) + bTeleportOffset) = bTeleport;
	}

	Mesh->ProcessEvent(K2_SetWorldTransformFn, K2_SetWorldTransformParams);
	// Mesh->bComponentToWorldUpdated = true;

	struct { FVector NewVel; bool bAddToCurrent; FName BoneName; } 
	UPrimitiveComponent_SetPhysicsLinearVelocity_Params{
		*(FVector*)(__int64(State) + LinearVelocityOffset),
		0,
		FName()
	};

	struct { FVector NewAngVel; bool bAddToCurrent; FName BoneName; }
	UPrimitiveComponent_SetPhysicsAngularVelocity_Params{
		*(FVector*)(__int64(State) + AngularVelocityOffset),
		0,
		FName()
	};

	Mesh->ProcessEvent(SetPhysicsLinearVelocityFn, &UPrimitiveComponent_SetPhysicsLinearVelocity_Params);
	Mesh->ProcessEvent(SetPhysicsAngularVelocityFn, &UPrimitiveComponent_SetPhysicsAngularVelocity_Params);
}

static inline void AddVehicleHook()
{
	static auto FortPhysicsPawnDefault = FindObject("/Script/FortniteGame.Default__FortPhysicsPawn");

	if (FortPhysicsPawnDefault)
	{
		Hooking::MinHook::Hook(FortPhysicsPawnDefault, FindObject<UFunction>("/Script/FortniteGame.FortPhysicsPawn.ServerMove"),
			ServerVehicleUpdate, nullptr, false, true);
	}
}

static inline void SpawnVehicles()
{
	static auto FortAthenaVehicleSpawnerClass = FindObject<UClass>("/Script/FortniteGame.FortAthenaVehicleSpawner");
	TArray<AActor*> AllVehicleSpawners = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortAthenaVehicleSpawnerClass);

	for (int i = 0; i < AllVehicleSpawners.Num(); i++)
	{
		auto VehicleSpawner = (AllVehicleSpawners.at(i));

		static auto FortVehicleItemDefVariantsOffset = VehicleSpawner->GetOffset("FortVehicleItemDefVariants", false);

		bool aa = true;

		static auto VIDClass = FindObject<UClass>("/Script/FortniteGame.FortVehicleItemDefinition");

		if (FortVehicleItemDefVariantsOffset != 0)
		{
			struct FVehicleWeightedDef
			{
				TSoftObjectPtr<UFortItemDefinition> VehicleItemDef;
				char pad[0x20]; // FScalableFloat Weight;                                                   // 0x0028(0x0020) (Edit, BlueprintVisible, BlueprintReadOnly)
			};

			auto FortVehicleItemDefVariants = VehicleSpawner->GetPtr<TArray<FVehicleWeightedDef>>(FortVehicleItemDefVariantsOffset);

			if (FortVehicleItemDefVariants->Num() > 0)
			{
				aa = false;
				auto& first = FortVehicleItemDefVariants->At(0);

				auto AssetPathName = first.VehicleItemDef.SoftObjectPtr.ObjectID.AssetPathName;

				if (!AssetPathName.ComparisonIndex.Value)
					continue;

				auto VehicleItemDef = LoadObject(AssetPathName.ToString(), VIDClass);

				if (VehicleItemDef)
				{
					static auto VehicleActorClassOffset = VehicleItemDef->GetOffset("VehicleActorClass");

					auto VehicleActorClassSoft = VehicleItemDef->GetPtr<TSoftObjectPtr<UClass>>(VehicleActorClassOffset);

					auto VehicleClassAssetPath = VehicleActorClassSoft->SoftObjectPtr.ObjectID.AssetPathName;

					if (!VehicleClassAssetPath.ComparisonIndex.Value)
						continue;

					static auto BGAClass = FindObject<UClass>("/Script/Engine.BlueprintGeneratedClass");
					auto VehicleActorClass = LoadObject<UClass>(VehicleClassAssetPath.ToString(), BGAClass);
;
					if (!VehicleActorClass)
						continue;

					GetWorld()->SpawnActor<AActor>(VehicleActorClass, VehicleSpawner->GetActorLocation(), VehicleSpawner->GetActorRotation().Quaternion());
				}
			}
		}

		if (aa)
		{
			static auto FortVehicleItemDefOffset = VehicleSpawner->GetOffset("FortVehicleItemDef");

			auto FortVehicleItemDefSoft = VehicleSpawner->GetPtr<TSoftObjectPtr<UFortItemDefinition>>(FortVehicleItemDefOffset);

			auto FortVehicleItemDefAssetPath = FortVehicleItemDefSoft->SoftObjectPtr.ObjectID.AssetPathName;

			if (!FortVehicleItemDefAssetPath.ComparisonIndex.Value)
				continue;

			auto FortVehicleItemDef = LoadObject<UFortItemDefinition>(FortVehicleItemDefAssetPath.ToString(), VIDClass);

			if (!FortVehicleItemDef)
				continue;

			static auto VehicleActorClassOffset = FortVehicleItemDef->GetOffset("VehicleActorClass");

			auto VehicleActorClassSoft = FortVehicleItemDef->GetPtr<TSoftObjectPtr<UClass>>(VehicleActorClassOffset);

			auto VehicleActorClassAssetPath = VehicleActorClassSoft->SoftObjectPtr.ObjectID.AssetPathName;

			if (!VehicleActorClassAssetPath.ComparisonIndex.Value)
				continue;

			static auto BGAClass = FindObject<UClass>("/Script/Engine.BlueprintGeneratedClass");
			auto VehicleActorClass = LoadObject<UClass>(VehicleActorClassAssetPath.ToString(), BGAClass);

			if (!VehicleActorClass)
				continue;

			GetWorld()->SpawnActor<AActor>(VehicleActorClass, VehicleSpawner->GetActorLocation(), VehicleSpawner->GetActorRotation().Quaternion());
		}
	}

	AllVehicleSpawners.Free();
}
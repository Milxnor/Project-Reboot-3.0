#pragma once

#include "reboot.h"
#include "Stack.h"
#include "Actor.h"
#include "hooking.h"
#include "SoftObjectPtr.h"
#include "FortGameModeAthena.h"
#include "GameplayStatics.h"
#include "FortVehicleItemDefinition.h"
#include "FortDagwoodVehicle.h"

// Vehicle class name changes multiple times across versions, so I made it it's own file.

static inline void ServerVehicleUpdate(UObject* Context, FFrame& Stack, void* Ret)
{
	auto Params = Stack.Locals;

	auto Vehicle = Cast<AActor>(Context);
	static auto RootComponentOffset = Vehicle->GetOffset("RootComponent");
	auto Mesh = /* Cast<UPrimitiveComponent> */(Vehicle->Get(RootComponentOffset));

	FTransform Transform{};

	static std::string StateStructName = FindObject(L"/Script/FortniteGame.ReplicatedPhysicsPawnState") ? "/Script/FortniteGame.ReplicatedPhysicsPawnState" : "/Script/FortniteGame.ReplicatedAthenaVehiclePhysicsState";

	if (StateStructName.empty())
		return;

	auto StateStruct = FindObject(StateStructName);

	if (!StateStruct)
		return;

	auto State = (void*)(__int64(Params) + 0);

	static auto RotationOffset = FindOffsetStruct(StateStructName, "Rotation");
	static auto TranslationOffset = FindOffsetStruct(StateStructName, "Translation");

	if (Engine_Version >= 420) // S4-S12
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
	else
	{
		auto Rotation = (FQuat*)(__int64(State) + RotationOffset);

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

	VirtualFree(K2_SetWorldTransformParams, 0, MEM_RELEASE);

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
	static auto FortAthenaVehicleDefault = FindObject(L"/Script/FortniteGame.Default__FortAthenaVehicle");
	static auto FortPhysicsPawnDefault = FindObject(L"/Script/FortniteGame.Default__FortPhysicsPawn");

	if (FortPhysicsPawnDefault)
	{
		Hooking::MinHook::Hook(FortPhysicsPawnDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortPhysicsPawn.ServerMove") ?
			FindObject<UFunction>(L"/Script/FortniteGame.FortPhysicsPawn.ServerMove") : FindObject<UFunction>(L"/Script/FortniteGame.FortPhysicsPawn.ServerUpdatePhysicsParams"),
			ServerVehicleUpdate, nullptr, false, true);
	}
	else
	{
		Hooking::MinHook::Hook(FortAthenaVehicleDefault, FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaVehicle.ServerUpdatePhysicsParams"),
			ServerVehicleUpdate, nullptr, false, true);
	}
}

struct FVehicleWeightedDef
{
public:
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.VehicleWeightedDef");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	TSoftObjectPtr<UFortVehicleItemDefinition>* GetVehicleItemDef()
	{
		static auto VehicleItemDefOffset = FindOffsetStruct("/Script/FortniteGame.VehicleWeightedDef", "VehicleItemDef");
		return (TSoftObjectPtr<UFortVehicleItemDefinition>*)(__int64(this) + VehicleItemDefOffset);
	}

	FScalableFloat* GetWeight()
	{
		static auto WeightOffset = FindOffsetStruct("/Script/FortniteGame.VehicleWeightedDef", "Weight");
		return (FScalableFloat*)(__int64(this) + WeightOffset);
	}
};

static inline AActor* SpawnVehicleFromSpawner(AActor* VehicleSpawner)
{
	bool bDebugSpawnVehicles = false;

	auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

	FTransform SpawnTransform{};
	SpawnTransform.Translation = VehicleSpawner->GetActorLocation();
	SpawnTransform.Rotation = VehicleSpawner->GetActorRotation().Quaternion();
	SpawnTransform.Scale3D = { 1, 1, 1 };

	static auto VehicleClassOffset = VehicleSpawner->GetOffset("VehicleClass", false);
	static auto BGAClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");

	if (VehicleClassOffset != -1) // 10.40 and below?
	{
		auto& SoftVehicleClass = VehicleSpawner->Get<TSoftObjectPtr<UClass>>(VehicleClassOffset);
		auto StrongVehicleClass = SoftVehicleClass.Get(BGAClass, true);

		if (!StrongVehicleClass)
		{
			std::string VehicleClassObjectName = SoftVehicleClass.SoftObjectPtr.ObjectID.AssetPathName.ComparisonIndex.Value == 0 ? "InvalidName" : SoftVehicleClass.SoftObjectPtr.ObjectID.AssetPathName.ToString();
			LOG_WARN(LogVehicles, "Failed to load vehicle class: {}", VehicleClassObjectName);
			return nullptr;
		}

		if (bDebugSpawnVehicles)
			LOG_INFO(LogDev, "Spawning Vehicle: {}", StrongVehicleClass->GetPathName());

		return GetWorld()->SpawnActor<AActor>(StrongVehicleClass, SpawnTransform, CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));
	}

	static auto FortVehicleItemDefOffset = VehicleSpawner->GetOffset("FortVehicleItemDef");

	if (FortVehicleItemDefOffset == -1)
		return nullptr;

	auto& SoftFortVehicleItemDef = VehicleSpawner->Get<TSoftObjectPtr<UFortVehicleItemDefinition>>(FortVehicleItemDefOffset);
	UFortVehicleItemDefinition* VIDToSpawn = nullptr;

	if (SoftFortVehicleItemDef.SoftObjectPtr.ObjectID.AssetPathName.ComparisonIndex.Value == 0)
	{
		static auto FortVehicleItemDefVariantsOffset = VehicleSpawner->GetOffset("FortVehicleItemDefVariants");

		if (FortVehicleItemDefVariantsOffset != -1)
		{
			TArray<FVehicleWeightedDef>& FortVehicleItemDefVariants = VehicleSpawner->Get<TArray<FVehicleWeightedDef>>(FortVehicleItemDefVariantsOffset);

			if (FortVehicleItemDefVariants.size() > 0)
			{
				VIDToSpawn = FortVehicleItemDefVariants.at(0, FVehicleWeightedDef::GetStructSize()).GetVehicleItemDef()->Get(UFortVehicleItemDefinition::StaticClass(), true); // TODO (Milxnor) Implement the weight
			}
		}
	}
	else
	{
		VIDToSpawn = SoftFortVehicleItemDef.Get(UFortVehicleItemDefinition::StaticClass(), true);
	}

	if (!VIDToSpawn)
	{
		std::string FortVehicleItemDefObjectName = SoftFortVehicleItemDef.SoftObjectPtr.ObjectID.AssetPathName.ComparisonIndex.Value == 0 ? "InvalidName" : SoftFortVehicleItemDef.SoftObjectPtr.ObjectID.AssetPathName.ToString();
		LOG_WARN(LogVehicles, "Failed to load vehicle item definition: {}", FortVehicleItemDefObjectName);
		return nullptr;
	}

	UClass* StrongVehicleActorClass = VIDToSpawn->GetVehicleActorClass();

	if (!StrongVehicleActorClass)
	{
		std::string VehicleActorClassObjectName = VIDToSpawn->GetVehicleActorClassSoft()->SoftObjectPtr.ObjectID.AssetPathName.ComparisonIndex.Value == 0 ? "InvalidName" : VIDToSpawn->GetVehicleActorClassSoft()->SoftObjectPtr.ObjectID.AssetPathName.ToString();
		LOG_WARN(LogVehicles, "Failed to load vehicle actor class: {}", VehicleActorClassObjectName);
		return nullptr;
	}

	if (bDebugSpawnVehicles)
		LOG_INFO(LogDev, "Spawning Vehicle (VID): {}", StrongVehicleActorClass->GetPathName());

	auto NewVehicle = GetWorld()->SpawnActor<AActor>(StrongVehicleActorClass, SpawnTransform, CreateSpawnParameters(ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn));

	if (auto FortDagwoodVehicle = Cast<AFortDagwoodVehicle>(NewVehicle)) // carrr
	{
		FortDagwoodVehicle->SetFuel(100);
	}

	return NewVehicle;
}

static inline void SpawnVehicles2()
{
	static auto FortAthenaVehicleSpawnerClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaVehicleSpawner");
	TArray<AActor*> AllVehicleSpawners = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FortAthenaVehicleSpawnerClass);

	int AmountOfVehiclesSpawned = 0;

	for (int i = 0; i < AllVehicleSpawners.Num(); i++)
	{
		auto VehicleSpawner = AllVehicleSpawners.at(i);
		auto Vehicle = SpawnVehicleFromSpawner(VehicleSpawner);

		if (Vehicle)
		{
			AmountOfVehiclesSpawned++;
		}
	}

	auto AllVehicleSpawnersNum = AllVehicleSpawners.Num();

	AllVehicleSpawners.Free();

	LOG_INFO(LogGame, "Spawned {}/{} vehicles.", AmountOfVehiclesSpawned, AllVehicleSpawnersNum);
}
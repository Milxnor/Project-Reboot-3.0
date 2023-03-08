#pragma once

#include "EngineTypes.h"
#include "Transform.h"
#include "Object.h"
#include "Rotator.h"

struct FNetworkNotify
{

};

struct FActorSpawnParameters
{
	FName Name;
	UObject* Template;
	UObject* Owner;
	UObject** Instigator;
	UObject* OverrideLevel;
	ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride;
	uint16	bRemoteOwned : 1;
	uint16	bNoFail : 1;
	uint16	bDeferConstruction : 1;
	uint16	bAllowDuringConstructionScript : 1;
#if WITH_EDITOR
	uint16 bTemporaryEditorActor : 1;
#endif
	EObjectFlags ObjectFlags;
};

class UWorld : public UObject, public FNetworkNotify
{
public:
	static inline UObject* (*SpawnActorOriginal)(UWorld* World, UClass* Class, FTransform const* UserTransformPtr, const FActorSpawnParameters& SpawnParameters);

	template <typename T = AActor>
	T* GetGameMode()
	{
		static auto AuthorityGameModeOffset = GetOffset("AuthorityGameMode");
		return this->Get<T*>(AuthorityGameModeOffset);
	}

	template <typename ActorType>
	ActorType* SpawnActor(UClass* Class, FTransform UserTransformPtr = FTransform(), const FActorSpawnParameters& SpawnParameters = FActorSpawnParameters())
	{
		return (ActorType*)SpawnActorOriginal(this, Class, &UserTransformPtr, SpawnParameters);
	}

	template <typename ActorType>
	ActorType* SpawnActor(UClass* Class, FVector Location, FQuat Rotation = FQuat(), FVector Scale3D = FVector(1, 1, 1), const FActorSpawnParameters& SpawnParameters = FActorSpawnParameters())
	{
		FTransform UserTransformPtr{};
		UserTransformPtr.Translation = Location;
		UserTransformPtr.Rotation = Rotation;
		UserTransformPtr.Scale3D = Scale3D;
		return SpawnActor<ActorType>(Class, UserTransformPtr, SpawnParameters);
	}

	UObject* K2_GetWorldSettings();

	void Listen();
};
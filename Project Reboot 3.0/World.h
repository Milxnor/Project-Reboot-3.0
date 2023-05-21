#pragma once

#include "EngineTypes.h"
#include "Transform.h"
#include "Object.h"
#include "Rotator.h"
#include "Actor.h"
#include "GameInstance.h"

struct FNetworkNotify
{

};

class AWorldSettings : public AActor
{
public:
};

struct FActorSpawnParameters
{
	FName Name = FName(0);
	UObject* Template = nullptr;
	UObject* Owner = nullptr;
	UObject** Instigator = nullptr;
	UObject* OverrideLevel = nullptr;
	ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;
	uint16	bRemoteOwned : 1;
	uint16	bNoFail : 1;
	uint16	bDeferConstruction : 1;
	uint16	bAllowDuringConstructionScript : 1;
#if WITH_EDITOR
	uint16 bTemporaryEditorActor : 1;
#endif
	EObjectFlags ObjectFlags;
};

struct FActorSpawnParametersUE500
{
	FName Name = FName(0);
	UObject* Template = nullptr;
	UObject* Owner = nullptr;
	UObject** Instigator = nullptr;
	UObject* OverrideLevel = nullptr;
	UObject* OverrideParentComponent;
	ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;
	uint8_t TransformScaleMethod;
	uint16	bRemoteOwned : 1;
	uint16	bNoFail : 1;
	uint16	bDeferConstruction : 1;
	uint16	bAllowDuringConstructionScript : 1;
#if WITH_EDITOR
	uint16 bTemporaryEditorActor : 1;
#endif
	enum class ESpawnActorNameMode : uint8_t
	{
		Required_Fatal,
		Required_ErrorAndReturnNull,
		Required_ReturnNull,
		Requested
	};

	ESpawnActorNameMode NameMode;
	EObjectFlags ObjectFlags;
	TFunction<void(UObject*)> CustomPreSpawnInitalization; // my favorite
};

static inline void* CreateSpawnParameters(ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined, bool bDeferConstruction = false, UObject* Owner = nullptr)
{
	if (Engine_Version >= 500)
	{
		auto addr = (FActorSpawnParametersUE500*)VirtualAlloc(0, sizeof(FActorSpawnParametersUE500), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		if (!addr)
			return nullptr;

		addr->Owner = Owner;
		addr->bDeferConstruction = bDeferConstruction;
		addr->SpawnCollisionHandlingOverride = SpawnCollisionHandlingOverride;
		return addr;
	}
	else
	{
		auto addr = (FActorSpawnParameters*)VirtualAlloc(0, sizeof(FActorSpawnParameters), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		if (!addr)
			return nullptr;

		addr->Owner = Owner;
		addr->bDeferConstruction = bDeferConstruction;
		addr->SpawnCollisionHandlingOverride = SpawnCollisionHandlingOverride;
		return addr;
	}

	return nullptr;
}

class UWorld : public UObject, public FNetworkNotify
{
public:
	static inline UObject* (*SpawnActorOriginal)(UWorld* World, UClass* Class, FTransform const* UserTransformPtr, void* SpawnParameters);

	template <typename T = AActor>
	T*& GetGameMode()
	{
		static auto AuthorityGameModeOffset = GetOffset("AuthorityGameMode");
		return this->Get<T*>(AuthorityGameModeOffset);
	}

	class AGameState*& GetGameState()
	{
		static auto GameStateOffset = GetOffset("GameState");
		return this->Get<class AGameState*>(GameStateOffset);
	}

	class UNetDriver*& GetNetDriver()
	{
		static auto NetDriverOffset = GetOffset("NetDriver");
		return this->Get<class UNetDriver*>(NetDriverOffset);
	}

	UGameInstance* GetOwningGameInstance()
	{
		static auto OwningGameInstanceOffset = GetOffset("OwningGameInstance");
		return this->Get<UGameInstance*>(OwningGameInstanceOffset);
	}

	inline FTimerManager& GetTimerManager()
	{
		return GetOwningGameInstance()->GetTimerManager();
		// return (GetOwningGameInstance() ? GetOwningGameInstance()->GetTimerManager() : *TimerManager);
	}

	template <typename ActorType>
	ActorType* SpawnActor(UClass* Class, FTransform UserTransformPtr = FTransform(), void* SpawnParameters = nullptr)
	{
		if (!SpawnParameters)
			SpawnParameters = CreateSpawnParameters();

		auto actor = (ActorType*)SpawnActorOriginal(this, Class, &UserTransformPtr, SpawnParameters);

		VirtualFree(SpawnParameters, 0, MEM_RELEASE);

		return actor;
	}

	template <typename ActorType>
	ActorType* SpawnActor(UClass* Class, FVector Location, FQuat Rotation = FQuat(), FVector Scale3D = FVector(1, 1, 1), void* SpawnParameters = nullptr)
	{
		if (!SpawnParameters)
			SpawnParameters = CreateSpawnParameters();

		FTransform UserTransformPtr{};
		UserTransformPtr.Translation = Location;
		UserTransformPtr.Rotation = Rotation;
		UserTransformPtr.Scale3D = Scale3D;

		auto actor = SpawnActor<ActorType>(Class, UserTransformPtr, SpawnParameters);

		VirtualFree(SpawnParameters, 0, MEM_RELEASE);

		return actor;
	}

	AWorldSettings* GetWorldSettings(bool bCheckStreamingPersistent = false, bool bChecked = true) const;
	AWorldSettings* K2_GetWorldSettings(); // DONT USE WHEN POSSIBLE

	void Listen();
};
#pragma once

#include "Object.h"

enum class ENetDormancy : uint8_t
{
	DORM_Never = 0,
	DORM_Awake = 1,
	DORM_DormantAll = 2,
	DORM_DormantPartial = 3,
	DORM_Initial = 4,
	DORN_MAX = 5,
	ENetDormancy_MAX = 6
};

class AActor : public UObject
{
public:
	struct FTransform GetTransform();

	bool IsTearOff();
	/* FORCEINLINE */ ENetDormancy& GetNetDormancy();
	int32& GetNetTag();
	AActor* GetOwner();
	struct FVector GetActorLocation();
	struct FVector GetActorRightVector();
	void K2_DestroyActor();
	class UActorComponent* GetComponentByClass(class UClass* ComponentClass);
	float GetDistanceTo(AActor* OtherActor);
	struct FRotator GetActorRotation();
	void FlushNetDormancy();
	bool TeleportTo(const FVector& DestLocation, const FRotator& DestRotation);
	bool IsActorBeingDestroyed();
	bool IsNetStartup();
	bool IsAlwaysRelevant();
	bool UsesOwnerRelevancy();
	bool IsOnlyRelevantToOwner();
	bool CanBeDamaged();
	void SetCanBeDamaged(bool NewValue);
	void SetOwner(AActor* Owner);
	void ForceNetUpdate();
	bool IsNetStartupActor();
	bool IsPendingKillPending();
	float& GetNetUpdateFrequency();
	float& GetMinNetUpdateFrequency();
	const AActor* GetNetOwner() const;

	bool IsRelevancyOwnerFor(const AActor* ReplicatedActor, const AActor* ActorOwner, const AActor* ConnectionActor) const
	{
		 // we should call virtual function but eh
		return (ActorOwner == this);
	}

	static class UClass* StaticClass();
};
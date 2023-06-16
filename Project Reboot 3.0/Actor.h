#pragma once

#include "Object.h"
#include "anticheat.h"

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

	// class UWorld* GetWorld();
	bool HasAuthority();
	bool IsTearOff();
	/* FORCEINLINE */ ENetDormancy& GetNetDormancy();
	int32& GetNetTag();
	void SetNetDormancy(ENetDormancy Dormancy);
	AActor* GetOwner();
	struct FVector GetActorScale3D();
	struct FVector GetActorLocation();
	struct FVector GetActorForwardVector();
	struct FVector GetActorRightVector();
	struct FVector GetActorUpVector();
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
	void GetActorEyesViewPoint(FVector* OutLocation, FRotator* OutRotation) const;
	AActor* GetClosestActor(UClass* ActorClass, float DistMax, std::function<bool(AActor*)> AdditionalCheck = [&](AActor*) { return true; });

	bool IsRelevancyOwnerFor(const AActor* ReplicatedActor, const AActor* ActorOwner, const AActor* ConnectionActor) const
	{
		// we should call virtual function but eh
		// return (ActorOwner == this);

		static auto IsRelevancyOwnerForOffset = 0x428;
		bool (*IsRelevancyOwnerForOriginal)(const AActor* Actor, const AActor * ReplicatedActor, const AActor * ActorOwner, const AActor * ConnectionActor) =
			decltype(IsRelevancyOwnerForOriginal)(this->VFTable[IsRelevancyOwnerForOffset / 8]);

		return IsRelevancyOwnerForOriginal(this, ReplicatedActor, ActorOwner, ConnectionActor);
	}

	static class UClass* StaticClass();
};
#pragma once

#include "Object.h"
#include "anticheat.h"
#include "Vector.h"

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
	static inline void (*originalCallPreReplication)(AActor*, class UNetDriver*);
	struct FTransform GetTransform();

	bool GetNetDormancy(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, class UActorChannel* InChannel, float Time, bool bLowBandwidth);
	bool HasAuthority();
	bool IsTearOff();
	class ULevel* GetLevel() const;
	/* FORCEINLINE */ ENetDormancy& NetDormancy();
	int32& GetNetTag();
	void SetNetDormancy(ENetDormancy Dormancy);
	AActor* GetOwner();
	FName& GetNetDriverName();
	ENetRole& GetRemoteRole();
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
	UObject* AddComponentByClass(UClass* Class);
	float& GetNetUpdateFrequency();
	float& GetMinNetUpdateFrequency();
	const AActor* GetNetOwner() const;
	bool IsActorInitialized();
	void GetActorEyesViewPoint(FVector* OutLocation, FRotator* OutRotation) const;
	AActor* GetClosestActor(UClass* ActorClass, float DistMax, std::function<bool(AActor*)> AdditionalCheck = [&](AActor*) { return true; });

	bool IsRelevancyOwnerFor(const AActor* ReplicatedActor, const AActor* ActorOwner, const AActor* ConnectionActor) const // T(REP)
	{
		// return (ActorOwner == this);

		static auto IsRelevancyOwnerForOffset = 0x428;
		bool (*IsRelevancyOwnerForOriginal)(const AActor* Actor, const AActor * ReplicatedActor, const AActor * ActorOwner, const AActor * ConnectionActor) =
			decltype(IsRelevancyOwnerForOriginal)(this->VFTable[IsRelevancyOwnerForOffset / 8]);

		return IsRelevancyOwnerForOriginal(this, ReplicatedActor, ActorOwner, ConnectionActor);
	}

	static class UClass* StaticClass();
};
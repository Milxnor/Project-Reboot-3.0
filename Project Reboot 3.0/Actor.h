#pragma once

#include "Object.h"

class AActor : public UObject
{
public:
	struct FTransform GetTransform();

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

	static class UClass* StaticClass();
};
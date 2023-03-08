#pragma once

#include "Object.h"

class AActor : public UObject
{
public:
	struct FTransform GetTransform();

	AActor* GetOwner();
	struct FVector GetActorLocation();
	void K2_DestroyActor();
	class UActorComponent* GetComponentByClass(class UClass* ComponentClass);
};
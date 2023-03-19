#pragma once

#include "Object.h"
#include "Array.h"
#include "Actor.h"

class UGameplayStatics : public UObject
{
public:
    static TArray<AActor*> GetAllActorsOfClass(const UObject* WorldContextObject, UClass* ActorClass);
	static float GetTimeSeconds(UObject* WorldContextObject);
	static UObject* SpawnObject(UClass* ObjectClass, UObject* Outer);

	static UClass* StaticClass();
};
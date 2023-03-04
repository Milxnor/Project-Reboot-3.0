#pragma once

#include "Object.h"
#include "Array.h"
#include "Actor.h"

class UGameplayStatics : public UObject
{
public:
    static TArray<AActor*> GetAllActorsOfClass(const UObject* WorldContextObject, UClass* ActorClass);
	static float GetTimeSeconds(const UObject* WorldContextObject);

	static UClass* StaticClass();
};
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
	// static void OpenLevel(UObject* WorldContextObject, FName LevelName, bool bAbsolute, const FString& Options);
	static void RemovePlayer(class APlayerController* Player, bool bDestroyPawn);

	static UClass* StaticClass();
};
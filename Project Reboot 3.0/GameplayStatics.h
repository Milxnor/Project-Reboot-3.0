#pragma once

#include "Object.h"
#include "Array.h"
#include "Actor.h"
#include "LatentActionManager.h"
#include "EngineTypes.h"

class UGameplayStatics : public UObject
{
public:
    static TArray<AActor*> GetAllActorsOfClass(const UObject* WorldContextObject, UClass* ActorClass);
	static float GetTimeSeconds(UObject* WorldContextObject);
	
	static UObject* SpawnObject(UClass* ObjectClass, UObject* Outer);

	template <typename ObjectType>
	static ObjectType* SpawnObject(UClass* ObjectClass, UObject* Outer, bool bCheckType) 
	{
		auto Object = SpawnObject(ObjectClass, Outer);
		return bCheckType ? Cast<ObjectType>(Object) : (ObjectType*)Object;
	}

	// static void OpenLevel(UObject* WorldContextObject, FName LevelName, bool bAbsolute, const FString& Options);
	static void RemovePlayer(class APlayerController* Player, bool bDestroyPawn);
	static AActor* FinishSpawningActor(AActor* Actor, const FTransform& SpawnTransform);
	static AActor* BeginDeferredActorSpawnFromClass(const UObject* WorldContextObject, UClass* ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined, AActor* Owner = nullptr);
	static void LoadStreamLevel(UObject* WorldContextObject, FName LevelName, bool bMakeVisibleAfterLoad, bool bShouldBlockOnLoad, const FLatentActionInfo& LatentInfo);
	static void UnloadStreamLevel(UObject* WorldContextObject, FName LevelName, const FLatentActionInfo& LatentInfo, bool bShouldBlockOnUnload);

	static UClass* StaticClass();
};
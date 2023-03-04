#include "GameplayStatics.h"

#include "reboot.h"

TArray<AActor*> UGameplayStatics::GetAllActorsOfClass(const UObject* WorldContextObject, UClass* ActorClass)
{
	static auto fn = FindObject<UFunction>(L"/Script/Engine.GameplayStatics.GetAllActorsOfClass");

	TArray<AActor*> ahh;

	struct { const UObject* WorldContextObject; UClass* ActorClass; TArray<AActor*> OutActors; }
	UGameplayStatics_GetAllActorsOfClass_Params{ WorldContextObject, ActorClass, ahh };

	static auto defaultObj = StaticClass();
	defaultObj->ProcessEvent(fn, &UGameplayStatics_GetAllActorsOfClass_Params);

	return ahh;
}

float UGameplayStatics::GetTimeSeconds(const UObject* WorldContextObject)
{
	static auto fn = FindObject<UFunction>(L"/Script/Engine.GameplayStatics.GetTimeSeconds");

	float TimeSeconds = 0;

	static auto defaultObj = StaticClass();
	defaultObj->ProcessEvent(fn, &TimeSeconds);

	return TimeSeconds;
}

UClass* UGameplayStatics::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.GameplayStatics");
	return Class;
}
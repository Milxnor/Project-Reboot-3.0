#include "GameplayStatics.h"

#include "reboot.h"

TArray<AActor*> UGameplayStatics::GetAllActorsOfClass(const UObject* WorldContextObject, UClass* ActorClass)
{
	static auto fn = FindObject<UFunction>(L"/Script/Engine.GameplayStatics.GetAllActorsOfClass");

	struct { const UObject* WorldContextObject; UClass* ActorClass; TArray<AActor*> OutActors; }
	UGameplayStatics_GetAllActorsOfClass_Params{ WorldContextObject, ActorClass };

	static auto defaultObj = StaticClass();
	defaultObj->ProcessEvent(fn, &UGameplayStatics_GetAllActorsOfClass_Params);

	return UGameplayStatics_GetAllActorsOfClass_Params.OutActors;
}

float UGameplayStatics::GetTimeSeconds(UObject* WorldContextObject)
{
	static auto fn = FindObject<UFunction>(L"/Script/Engine.GameplayStatics.GetTimeSeconds");

	struct { UObject* WorldContextObject; float TimeSeconds; } GetTimeSecondsParams{WorldContextObject};

	static auto defaultObj = StaticClass();
	defaultObj->ProcessEvent(fn, &GetTimeSecondsParams);

	return GetTimeSecondsParams.TimeSeconds;
}

UObject* UGameplayStatics::SpawnObject(UClass* ObjectClass, UObject* Outer)
{
	static auto fn = FindObject<UFunction>("/Script/Engine.GameplayStatics.SpawnObject");

	struct
	{
		UClass* ObjectClass;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		UObject* Outer;                                                    // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		UObject* ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} UGameplayStatics_SpawnObject_Params{ObjectClass, Outer};

	static auto defaultObj = StaticClass();
	defaultObj->ProcessEvent(fn, &UGameplayStatics_SpawnObject_Params);

	return UGameplayStatics_SpawnObject_Params.ReturnValue;
}

UClass* UGameplayStatics::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.GameplayStatics");
	return Class;
}
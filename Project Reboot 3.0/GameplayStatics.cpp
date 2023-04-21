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

/* void UGameplayStatics::OpenLevel(UObject* WorldContextObject, FName LevelName, bool bAbsolute, const FString& Options)
{
	static auto fn = FindObject<UFunction>("/Script/Engine.GameplayStatics.OpenLevel");

	struct
	{
		UObject* WorldContextObject;                                       // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FName                                       LevelName;                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		bool                                               bAbsolute;                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FString                                     Options;                                                  // (Parm, ZeroConstructor, AdvancedDisplay, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} UGameplayStatics_OpenLevel_Params{WorldContextObject, LevelName, bAbsolute, Options};

	static auto defaultObj = StaticClass();
	defaultObj->ProcessEvent(fn, &UGameplayStatics_OpenLevel_Params);	
} */

void UGameplayStatics::RemovePlayer(APlayerController* Player, bool bDestroyPawn)
{
	static auto fn = FindObject<UFunction>("/Script/Engine.GameplayStatics.RemovePlayer");

	struct
	{
		APlayerController* Player;                                                   // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		bool                                               bDestroyPawn;                                             // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} UGameplayStatics_RemovePlayer_Params{Player, bDestroyPawn};

	static auto defaultObj = StaticClass();
	defaultObj->ProcessEvent(fn, &UGameplayStatics_RemovePlayer_Params);
}

void UGameplayStatics::LoadStreamLevel(UObject* WorldContextObject, FName LevelName, bool bMakeVisibleAfterLoad, bool bShouldBlockOnLoad, const FLatentActionInfo& LatentInfo)
{
	static auto LoadStreamLevelFn = FindObject<UFunction>("/Script/Engine.GameplayStatics.LoadStreamLevel");

	struct
	{
		UObject* WorldContextObject;                                       // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FName                                       LevelName;                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		bool                                               bMakeVisibleAfterLoad;                                    // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		bool                                               bShouldBlockOnLoad;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FLatentActionInfo                           LatentInfo;                                               // (Parm, NoDestructor, NativeAccessSpecifierPublic)
	} UGameplayStatics_LoadStreamLevel_Params{WorldContextObject, LevelName, bMakeVisibleAfterLoad, bShouldBlockOnLoad, LatentInfo};

	static auto defaultObj = StaticClass();
	defaultObj->ProcessEvent(LoadStreamLevelFn, &UGameplayStatics_LoadStreamLevel_Params);
}

void UGameplayStatics::UnloadStreamLevel(UObject* WorldContextObject, FName LevelName, const FLatentActionInfo& LatentInfo, bool bShouldBlockOnUnload)
{
	static auto UnloadStreamLevelFn = FindObject<UFunction>("/Script/Engine.GameplayStatics.UnloadStreamLevel");
	struct
	{
		UObject* WorldContextObject;                                       // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FName                                       LevelName;                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FLatentActionInfo                           LatentInfo;                                               // (Parm, NoDestructor, NativeAccessSpecifierPublic)
		bool                                               bShouldBlockOnUnload;                                     // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} UGameplayStatics_UnloadStreamLevel_Params{WorldContextObject, LevelName, LatentInfo, bShouldBlockOnUnload};

	static auto defaultObj = StaticClass();
	defaultObj->ProcessEvent(UnloadStreamLevelFn, &UGameplayStatics_UnloadStreamLevel_Params);
}

UClass* UGameplayStatics::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.GameplayStatics");
	return Class;
}
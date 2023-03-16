#pragma once

#include "Object.h"
#include "UnrealString.h"

#include "reboot.h"

class UKismetSystemLibrary : public UObject
{
public:
	static FString GetPathName(UObject* Object)
	{
		static auto GetPathNameFunction = FindObject<UFunction>("/Script/Engine.KismetSystemLibrary.GetPathName");
		static auto KismetSystemLibrary = FindObject("/Script/Engine.Default__KismetSystemLibrary");

		struct { UObject* Object; FString ReturnValue; } GetPathName_Params{ Object };

		KismetSystemLibrary->ProcessEvent(GetPathNameFunction, &GetPathName_Params);

		auto Ret = GetPathName_Params.ReturnValue;

		return Ret;
	}

	static void ExecuteConsoleCommand(UObject* WorldContextObject, const FString& Command, class APlayerController* SpecificPlayer)
	{
		static auto KismetSystemLibrary = FindObject("/Script/Engine.Default__KismetSystemLibrary");
		static auto fn = FindObject<UFunction>("/Script/Engine.KismetSystemLibrary.ExecuteConsoleCommand");

		struct {
			UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FString                                     Command;                                                  // (Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			class APlayerController* SpecificPlayer;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		} UKismetSystemLibrary_ExecuteConsoleCommand_Params{WorldContextObject, Command, SpecificPlayer};

		KismetSystemLibrary->ProcessEvent(fn, &UKismetSystemLibrary_ExecuteConsoleCommand_Params);
	}
};
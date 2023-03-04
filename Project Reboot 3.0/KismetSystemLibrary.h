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
};
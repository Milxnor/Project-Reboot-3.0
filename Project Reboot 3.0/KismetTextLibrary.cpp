#include "KismetTextLibrary.h"

#include "reboot.h"

FText UKismetTextLibrary::Conv_StringToText(const FString& inString)
{
	static auto Conv_StringToTextFn = FindObject<UFunction>("/Script/Engine.KismetTextLibrary.Conv_StringToText");

	struct
	{
		FString                                     inString;                                                 // (Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FText                                       ReturnValue;                                              // (Parm, OutParm, ReturnParm, NativeAccessSpecifierPublic)
	}UKismetTextLibrary_Conv_StringToText_Params{inString};

	static auto Default__KismetTextLibrary = FindObject<UKismetTextLibrary>("/Script/Engine.Default__KismetTextLibrary");
	Default__KismetTextLibrary->ProcessEvent(Conv_StringToTextFn, &UKismetTextLibrary_Conv_StringToText_Params);

	return UKismetTextLibrary_Conv_StringToText_Params.ReturnValue;
}

UClass* UKismetTextLibrary::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/Engine.KismetTextLibrary");
	return Class;
}
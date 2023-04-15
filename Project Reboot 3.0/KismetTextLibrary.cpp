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

FString UKismetTextLibrary::Conv_TextToString(FText InText)
{
	static auto Conv_TextToStringFn = FindObject<UFunction>("/Script/Engine.KismetTextLibrary.Conv_TextToString");

	struct
	{
		FText                                       InText;                                                   // (ConstParm, Parm, OutParm, ReferenceParm, NativeAccessSpecifierPublic)
		FString                                     ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} UKismetTextLibrary_Conv_TextToString_Params{InText};

	static auto Default__KismetTextLibrary = FindObject<UKismetTextLibrary>("/Script/Engine.Default__KismetTextLibrary");
	Default__KismetTextLibrary->ProcessEvent(Conv_TextToStringFn, &UKismetTextLibrary_Conv_TextToString_Params);

	return UKismetTextLibrary_Conv_TextToString_Params.ReturnValue;
}

UClass* UKismetTextLibrary::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/Engine.KismetTextLibrary");
	return Class;
}
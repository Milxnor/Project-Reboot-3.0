#include "KismetStringLibrary.h"

#include "reboot.h"

FName UKismetStringLibrary::Conv_StringToName(const FString& InString)
{
	static auto Conv_StringToName = FindObject<UFunction>(L"/Script/Engine.KismetStringLibrary.Conv_StringToName");
	static auto Default__KismetStringLibrary = FindObject<UKismetStringLibrary>(L"/Script/Engine.Default__KismetStringLibrary");

	struct { FString InString; FName ReturnValue; } Conv_StringToName_Params{ InString };

	Default__KismetStringLibrary->ProcessEvent(Conv_StringToName, &Conv_StringToName_Params);

	return Conv_StringToName_Params.ReturnValue;
}
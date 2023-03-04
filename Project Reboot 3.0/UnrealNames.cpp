#include "NameTypes.h"

#include "reboot.h"
#include "UnrealString.h"

#include "KismetStringLibrary.h"

std::string FName::ToString()
{
	static auto KismetStringLibrary = FindObject<UKismetStringLibrary>(L"/Script/Engine.Default__KismetStringLibrary");

	static auto Conv_NameToString = FindObject<UFunction>(L"/Script/Engine.KismetStringLibrary.Conv_NameToString");

	struct { FName InName; FString OutStr; } Conv_NameToString_Params{ *this };

	KismetStringLibrary->ProcessEvent(Conv_NameToString, &Conv_NameToString_Params);

	auto Str = Conv_NameToString_Params.OutStr.ToString();
	
	// Conv_NameToString_Params.OutStr.Free();

	return Str;
}
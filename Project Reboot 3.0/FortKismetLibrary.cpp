#include "FortKismetLibrary.h"

UFortResourceItemDefinition* UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType ResourceType)
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.K2_GetResourceItemDefinition");

	struct { EFortResourceType type; UFortResourceItemDefinition* ret; } params{ResourceType};
	
	static auto DefaultClass = StaticClass();
	DefaultClass->ProcessEvent(fn, &params);
	return params.ret;
}

void UFortKismetLibrary::ApplyCharacterCosmetics(UObject* WorldContextObject, const TArray<UObject*>& CharacterParts, UObject* PlayerState, bool* bSuccess)
{
	static auto fn = FindObject<UFunction>("/Script/FortniteGame.FortKismetLibrary.ApplyCharacterCosmetics");

	struct
	{
		UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		TArray<UObject*>                CharacterParts;                                           // (Parm, ZeroConstructor, NativeAccessSpecifierPublic)
		UObject* PlayerState;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		bool                                               bSuccess;                                                 // (Parm, OutParm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} UFortKismetLibrary_ApplyCharacterCosmetics_Params{WorldContextObject, CharacterParts, PlayerState };

	static auto DefaultClass = StaticClass();
	DefaultClass->ProcessEvent(fn, &UFortKismetLibrary_ApplyCharacterCosmetics_Params);

	if (bSuccess)
		*bSuccess = UFortKismetLibrary_ApplyCharacterCosmetics_Params.bSuccess;
}

UClass* UFortKismetLibrary::StaticClass()
{
	static auto ptr = FindObject<UClass>(L"/Script/FortniteGame.FortKismetLibrary");
	return ptr;
}
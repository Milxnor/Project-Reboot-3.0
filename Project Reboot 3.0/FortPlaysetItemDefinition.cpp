#include "FortPlaysetItemDefinition.h"

#include "PlaysetLevelStreamComponent.h"

void UFortPlaysetItemDefinition::ShowPlayset(UFortPlaysetItemDefinition* PlaysetItemDef, AFortVolume* Volume)
{
	/* This doesn't stream it to the client.
	
	static auto SpawnPlaysetFn = FindObject<UFunction>("/Script/FortniteGame.FortPlaysetItemDefinition.SpawnPlayset");

	struct
	{
		UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		UFortPlaysetItemDefinition* Playset;                                                  // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FVector                                     Location;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FRotator                                    Rotation;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
		char pad[0x100]; // idk out param stuff
	} UFortPlaysetItemDefinition_SpawnPlayset_Params{GetWorld(), PlaysetItemDef, Volume->GetActorLocation(), FRotator()};

	static auto defaultObj = FindObject<UFortPlaysetItemDefinition>("/Script/FortniteGame.Default__FortPlaysetItemDefinition");
	defaultObj->ProcessEvent(SpawnPlaysetFn, &UFortPlaysetItemDefinition_SpawnPlayset_Params);

	return; */

	auto VolumeToUse = Volume;

	static auto PlaysetLevelStreamComponentClass = FindObject<UClass>("/Script/FortniteGame.PlaysetLevelStreamComponent");
	auto LevelStreamComponent = (UPlaysetLevelStreamComponent*)VolumeToUse->GetComponentByClass(PlaysetLevelStreamComponentClass);

	if (!LevelStreamComponent)
	{
		return;
	}

	static auto SetPlaysetFn = FindObject<UFunction>("/Script/FortniteGame.PlaysetLevelStreamComponent.SetPlayset");
	LevelStreamComponent->ProcessEvent(SetPlaysetFn, &PlaysetItemDef);

	LoadPlaysetOriginal(LevelStreamComponent);
}
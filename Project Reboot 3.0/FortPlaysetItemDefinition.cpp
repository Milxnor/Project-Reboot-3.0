#include "FortPlaysetItemDefinition.h"

void UFortPlaysetItemDefinition::ShowPlayset(UFortPlaysetItemDefinition* PlaysetItemDef, AFortVolume* Volume)
{
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
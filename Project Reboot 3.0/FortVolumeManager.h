#pragma once

#include "FortVolume.h"

#include "FortPlaysetItemDefinition.h"

class AFortVolumeManager : public AActor
{
public:
	static inline AFortVolume* (*SpawnVolumeOriginal)(AFortVolumeManager* VolumeManager, UClass* VolumeActor, UFortPlaysetItemDefinition* Playset, FVector Location, FRotator Rotation);

	static AFortVolume* SpawnVolumeHook(AFortVolumeManager* VolumeManager, UClass* VolumeActor, UFortPlaysetItemDefinition* Playset, FVector Location, FRotator Rotation);
};
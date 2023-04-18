#pragma once

#include "ActorComponent.h"

#include "FortPlaysetItemDefinition.h"

class UPlaysetLevelStreamComponent : public UActorComponent
{
public:
	static inline void (*SetPlaysetOriginal)(UPlaysetLevelStreamComponent* Component, UFortPlaysetItemDefinition* NewPlayset);

	static void SetPlaysetHook(UPlaysetLevelStreamComponent* Component, UFortPlaysetItemDefinition* NewPlayset);
};
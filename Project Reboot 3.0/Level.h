#pragma once

#include "World.h"

class ULevel : public UObject
{
public:
	UWorld*& GetOwningWorld();
	bool HasVisibilityChangeRequestPending();
	AWorldSettings* GetWorldSettings(bool bChecked = true) const;
};
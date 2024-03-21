#pragma once

#include "World.h"

class ULevel : public UObject
{
public:
	UWorld*& GetOwningWorld();
	bool HasVisibilityChangeRequestPending();
	bool IsAssociatingLevel();
	AWorldSettings* GetWorldSettings(bool bChecked = true) const;

	static class UClass* StaticClass();
};
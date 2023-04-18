#pragma once

#include "Object.h"

class UCheatManager : public UObject
{
public:
	void Teleport();

	static UClass* StaticClass();
};
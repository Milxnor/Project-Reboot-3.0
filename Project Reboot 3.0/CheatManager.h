#pragma once

#include "Object.h"

class UCheatManager : public UObject
{
public:
	void Teleport();
	void DestroyTarget();

	static UClass* StaticClass();
};
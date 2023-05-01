#pragma once

#include "FortAthenaMutator.h"

#include "reboot.h"

class AFortAthenaMutator_TDM : public AFortAthenaMutator
{
public:
	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaMutator_TDM");
		return Class;
	}
};
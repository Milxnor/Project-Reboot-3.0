// this file wasn't fun

#pragma once

#include "reboot.h"

struct FUniqueNetIdRepl // : public FUniqueNetIdWrapper
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/Engine.UniqueNetIdRepl");
		return Struct;
	}

	static int GetSizeOfStruct()
	{
		static auto Size = GetStruct()->GetPropertiesSize();
		return Size;
	}

	/* bool IsEqual(FUniqueNetIdRepl* Other) */
};
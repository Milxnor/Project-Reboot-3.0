#pragma once

#include "FortPlayerPawn.h"

class AFortPlayerPawnAthena : public AFortPlayerPawn
{
public:
	static inline void (*OnCapsuleBeginOverlapOriginal)(UObject* Context, FFrame* Stack, void* Ret);

	uint8& GetDBNORevivalStacking()
	{
		static auto DBNORevivalStackingOffset = GetOffset("DBNORevivalStacking");
		return Get<uint8>(DBNORevivalStackingOffset);
	}

	static void OnCapsuleBeginOverlapHook(UObject* Context, FFrame* Stack, void* Ret);
};
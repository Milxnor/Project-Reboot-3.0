#pragma once

#include "FortPlayerPawn.h"

class AFortPlayerPawnAthena : public AFortPlayerPawn
{
public:
	static inline void (*OnCapsuleBeginOverlapOriginal)(UObject* Context, FFrame* Stack, void* Ret);

	static void OnCapsuleBeginOverlapHook(UObject* Context, FFrame* Stack, void* Ret);
};
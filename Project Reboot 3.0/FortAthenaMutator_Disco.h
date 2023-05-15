// Disco Domination

#pragma once

#include "FortAthenaMutator.h"
#include "Array.h"

struct FControlPointSpawnData
{

};

class AFortAthenaMutator_Disco : public AFortAthenaMutator
{
public:
	static inline void (*OnGamePhaseStepChangedOriginal)(UObject* Context, FFrame& Stack, void* Ret);

	TArray<FControlPointSpawnData>& GetControlPointSpawnData()
	{
		static auto ControlPointSpawnDataOffset = GetOffset("ControlPointSpawnData");
		return Get<TArray<FControlPointSpawnData>>(ControlPointSpawnDataOffset);
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaMutator_Disco");
		return Class;
	}

	static void OnGamePhaseStepChangedHook(UObject* Context, FFrame& Stack, void* Ret);
};
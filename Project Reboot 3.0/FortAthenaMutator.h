#pragma once

#include <functional>

#include "Actor.h"

#include "Stack.h"

#include "ScriptInterface.h"
#include "FortGameStateAthena.h"
#include "GameplayStatics.h"

class AFortAthenaMutator : public AActor // AFortGameplayMutator
{
public:
	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaMutator");
		return Class;
	}
};

static inline void LoopMutators(std::function<void(AFortAthenaMutator*)> Callback)
{
	auto AllMutators = UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFortAthenaMutator::StaticClass());

	for (int i = 0; i < AllMutators.Num(); i++)
	{
		Callback((AFortAthenaMutator*)AllMutators.at(i));
	}

	AllMutators.Free();
}
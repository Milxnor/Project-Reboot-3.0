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

template <typename MutatorType = AFortAthenaMutator>
static inline MutatorType* FindFirstMutator(UClass* MutatorClass = MutatorType::StaticClass())
{
	auto AllMutators = UGameplayStatics::GetAllActorsOfClass(GetWorld(), MutatorClass);
	auto FirstMutator = AllMutators.Num() >= 1 ? AllMutators.at(0) : nullptr;

	AllMutators.Free();

	return (MutatorType*)FirstMutator;
}
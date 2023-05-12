#include "BuildingSMActor.h"

void ABuildingSMActor::BuildingSMActor_SetMeshSet(FMeshSet* MeshSet)
{
	static auto BreakEffectOffset = GetOffset("BreakEffect");
	Get<UParticleSystem*>(BreakEffectOffset) = MeshSet->GetBreakEffect();

	static auto ConstructedEffectOffset = GetOffset("ConstructedEffect");
	Get<UParticleSystem*>(ConstructedEffectOffset) = MeshSet->GetConstructedEffect();
}

UClass* ABuildingSMActor::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.BuildingSMActor");
	return Class;
}
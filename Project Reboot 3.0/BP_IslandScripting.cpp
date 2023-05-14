#include "BP_IslandScripting.h"

void ABP_IslandScripting_C::Initialize()
{
	static auto UpdateMapOffset = GetOffset("UpdateMap", false);

	if (UpdateMapOffset != -1)
	{
		Get<bool>(UpdateMapOffset) = true;
		this->OnRep_UpdateMap();
	}

	/*

	// This spawns the beam.
	
	this->IsDeimosActive() = true;
	this->OnRep_IsDeimosActive();
	*/
}

ABP_IslandScripting_C* ABP_IslandScripting_C::GetIslandScripting()
{
	auto AllIslandScriptings = UGameplayStatics::GetAllActorsOfClass(GetWorld(), StaticClass());
	return AllIslandScriptings.Num() > 0 ? (ABP_IslandScripting_C*)AllIslandScriptings.at(0) : nullptr;
}
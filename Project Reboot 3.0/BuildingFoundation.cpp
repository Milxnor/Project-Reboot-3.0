#include "BuildingFoundation.h"
#include "FortGameModeAthena.h"

void ABuildingFoundation::SetDynamicFoundationTransformHook(UObject* Context, FFrame& Stack, void* Ret)
{
	FTransform NewTransform;
	Stack.StepCompiledIn(&NewTransform);

	auto BuildingFoundation = (ABuildingFoundation*)Context;

	LOG_INFO(LogDev, "Bruh: {}", BuildingFoundation->GetName());

	SetFoundationTransform(BuildingFoundation, NewTransform);

	return SetDynamicFoundationTransformOriginal(Context, Stack, Ret);
}

void ABuildingFoundation::SetDynamicFoundationEnabledHook(UObject* Context, FFrame& Stack, void* Ret)
{
	bool bEnabled;
	Stack.StepCompiledIn(&bEnabled);

	// LOG_INFO(LogDev, "{} TELL MILXNOR IF THIS PRINTS: {}", Context->GetFullName(), bEnabled);

	auto BuildingFoundation = (ABuildingFoundation*)Context;

	ShowFoundation(BuildingFoundation, bEnabled);

	return SetDynamicFoundationEnabledOriginal(Context, Stack, Ret);
}
#include "FortAthenaMutator_Disco.h"

void AFortAthenaMutator_Disco::OnGamePhaseStepChangedHook(UObject* Context, FFrame& Stack, void* Ret)
{
	/* TScriptInterface<UObject> SafeZoneInterface;
	EAthenaGamePhaseStep GamePhaseStep = EAthenaGamePhaseStep::BusFlying;

	static auto SafeZoneInterfaceOffset = FindOffsetStruct("/Script/FortniteGame.FortAthenaMutator_Disco.OnGamePhaseStepChanged", "SafeZoneInterface", false);

	if (SafeZoneInterfaceOffset != -1)
		Stack.StepCompiledIn(&SafeZoneInterface);

	Stack.StepCompiledIn(&GamePhaseStep, true); */

	// LOG_INFO(LogDev, "{} GamePhaseStep: {}", __FUNCTION__, (int)GamePhaseStep);

	return OnGamePhaseStepChangedOriginal(Context, Stack, Ret);
}
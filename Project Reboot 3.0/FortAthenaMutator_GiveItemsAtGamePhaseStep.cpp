#include "FortAthenaMutator_GiveItemsAtGamePhaseStep.h"

void AFortAthenaMutator_GiveItemsAtGamePhaseStep::OnGamePhaseStepChangedHook(UObject* Context, FFrame& Stack, void* Ret)
{
	LOG_INFO(LogDev, __FUNCTION__);

	return OnGamePhaseStepChangedOriginal(Context, Stack, Ret);
}
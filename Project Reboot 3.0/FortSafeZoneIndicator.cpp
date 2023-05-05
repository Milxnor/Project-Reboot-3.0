#include "FortSafeZoneIndicator.h"

#include "FortGameModeAthena.h"
#include "reboot.h"

void AFortSafeZoneIndicator::SkipShrinkSafeZone()
{
	auto GameState = Cast<AFortGameStateAthena>(((AFortGameMode*)GetWorld()->GetGameMode())->GetGameState());

	GetSafeZoneStartShrinkTime() = GameState->GetServerWorldTimeSeconds();
	GetSafeZoneFinishShrinkTime() = GameState->GetServerWorldTimeSeconds() + 0.2;
}

void AFortSafeZoneIndicator::OnSafeZoneStateChangeHook(AFortSafeZoneIndicator* SafeZoneIndicator, EFortSafeZoneState NewState, bool bInitial)
{
	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	LOG_INFO(LogDev, "OnSafeZoneStateChangeHook!");

	if (NewState == EFortSafeZoneState::Shrinking)
	{
		GameState->SetGamePhaseStep(EAthenaGamePhaseStep::StormShrinking);
	}
	else if (NewState == EFortSafeZoneState::Holding)
	{
		GameState->SetGamePhaseStep(EAthenaGamePhaseStep::StormHolding);
	}

	return OnSafeZoneStateChangeOriginal(SafeZoneIndicator, NewState, bInitial);
}
#include "FortSafeZoneIndicator.h"

#include "FortGameModeAthena.h"
#include "reboot.h"
#include "KismetSystemLibrary.h"

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

	return OnSafeZoneStateChangeOriginal(SafeZoneIndicator, NewState, bInitial);
}
#include "FortSafeZoneIndicator.h"

#include "FortGameModeAthena.h"
#include "reboot.h"

void AFortSafeZoneIndicator::SkipShrinkSafeZone()
{
	auto GameState = Cast<AFortGameStateAthena>(((AFortGameMode*)GetWorld()->GetGameMode())->GetGameState());

	GetSafeZoneStartShrinkTime() = GameState->GetServerWorldTimeSeconds();
	GetSafeZoneFinishShrinkTime() = GameState->GetServerWorldTimeSeconds() + 0.2;
}
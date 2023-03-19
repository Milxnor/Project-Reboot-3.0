#pragma once

#include "reboot.h"
#include "FortGameModeAthena.h"
#include "GameplayStatics.h"

static inline void (*SetZoneToIndexOriginal)(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK);

static void SetZoneToIndexHook(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK)
{
	static auto SafeZoneIndicatorOffset = GameModeAthena->GetOffset("SafeZoneIndicator");
	auto SafeZoneIndicator = GameModeAthena->Get<AActor*>(SafeZoneIndicatorOffset);

	static auto SafeZoneFinishShrinkTimeOffset = SafeZoneIndicator->GetOffset("SafeZoneFinishShrinkTime");
	static auto SafeZoneStartShrinkTimeOffset = SafeZoneIndicator->GetOffset("SafeZoneStartShrinkTime");

	static auto SafeZonePhaseOffset = GameModeAthena->GetOffset("SafeZonePhase");
	auto SafeZonePhase = GameModeAthena->Get<int>(SafeZonePhaseOffset);
	
	LOG_INFO(LogDev, "aa");
	LOG_INFO(LogZone, "SafeZonePhase: {}", SafeZonePhase);
	LOG_INFO(LogZone, "OverridePhaseMaybeIDFK: {}", OverridePhaseMaybeIDFK);
	LOG_INFO(LogZone, "SafeZoneFinishShrinkTime Before Call: {}", SafeZoneIndicator->Get<float>(SafeZoneFinishShrinkTimeOffset));
	LOG_INFO(LogZone, "SafeZoneStartShrinkTime Before Call: {}", SafeZoneIndicator->Get<float>(SafeZoneStartShrinkTimeOffset));
	LOG_INFO(LogZone, "TimeSeconds: {}", UGameplayStatics::GetTimeSeconds(GetWorld()));

	SafeZoneIndicator->Get<float>(SafeZoneStartShrinkTimeOffset) = UGameplayStatics::GetTimeSeconds(GetWorld());
	SafeZoneIndicator->Get<float>(SafeZoneFinishShrinkTimeOffset) = SafeZoneIndicator->Get<float>(SafeZoneStartShrinkTimeOffset) + 100;

	SetZoneToIndexOriginal(GameModeAthena, OverridePhaseMaybeIDFK);

	static auto NextMegaStormGridCellThicknessOffset = SafeZoneIndicator->GetOffset("NextMegaStormGridCellThickness");
	auto NextMegaStormGridCellThickness = SafeZoneIndicator->Get<float>(NextMegaStormGridCellThicknessOffset);

	LOG_INFO(LogZone, "NextMegaStormGridCellThickness: {}", NextMegaStormGridCellThickness);
	LOG_INFO(LogZone, "SafeZoneFinishShrinkTime After Call: {}", SafeZoneIndicator->Get<float>(SafeZoneFinishShrinkTimeOffset));
	LOG_INFO(LogZone, "SafeZoneStartShrinkTime After Call: {}", SafeZoneIndicator->Get<float>(SafeZoneStartShrinkTimeOffset));
}
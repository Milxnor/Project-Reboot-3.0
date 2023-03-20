#pragma once

#include "reboot.h"
#include "FortGameModeAthena.h"
#include "GameplayStatics.h"
#include "CurveTable.h"
#include "KismetStringLibrary.h"s
#include "DataTableFunctionLibrary.h"

static inline void (*SetZoneToIndexOriginal)(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK);

static void SetZoneToIndexHook(AFortGameModeAthena* GameModeAthena, int OverridePhaseMaybeIDFK)
{
	auto GameState = Cast<AFortGameStateAthena>(GameModeAthena->GetGameState());

	if (!GameState)
		return;

	static auto SafeZoneIndicatorOffset = GameModeAthena->GetOffset("SafeZoneIndicator");
	auto SafeZoneIndicator = GameModeAthena->Get<AActor*>(SafeZoneIndicatorOffset);

	static auto SafeZoneFinishShrinkTimeOffset = SafeZoneIndicator->GetOffset("SafeZoneFinishShrinkTime");
	static auto SafeZoneStartShrinkTimeOffset = SafeZoneIndicator->GetOffset("SafeZoneStartShrinkTime");

	static auto SafeZonePhaseOffset = GameModeAthena->GetOffset("SafeZonePhase");

	static auto MapInfoOffset = GameState->GetOffset("MapInfo");
	auto MapInfo = GameState->Get<AActor*>(MapInfoOffset);

	static auto SafeZoneDefinitionOffset = MapInfo->GetOffset("SafeZoneDefinition");
	auto SafeZoneDefinition = MapInfo->GetPtr<__int64>(SafeZoneDefinitionOffset);

	static auto ZoneDurationsOffset = 0x1F8;
	static auto ZoneHoldDurationsOffset = ZoneDurationsOffset - 0x10;

	auto& ZoneDurations = *(TArray<float>*)(__int64(SafeZoneDefinition) + ZoneDurationsOffset);
	auto& ZoneHoldDurations = *(TArray<float>*)(__int64(SafeZoneDefinition) + ZoneHoldDurationsOffset);

	static bool bFilledDurations = false;

	if (!bFilledDurations)
	{
		bFilledDurations = true;

		auto CurrentPlaylist = GetPlaylistToUse();
		UCurveTable* FortGameData = nullptr;

		static auto GameDataOffset = CurrentPlaylist->GetOffset("GameData");
		FortGameData = CurrentPlaylist ? CurrentPlaylist->Get<TSoftObjectPtr<UCurveTable>>(GameDataOffset).Get() : nullptr;

		if (!FortGameData)
			FortGameData = FindObject<UCurveTable>("/Game/Balance/AthenaGameData.AthenaGameData");

		auto ShrinkTimeFName = UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.ShrinkTime");
		FString ContextString;

		/* for (float i = 0; i < 1.1; i += 0.1)
		{
			float res;
			UDataTableFunctionLibrary::EvaluateCurveTableRow(FortGameData, ShrinkTimeFName, i, ContextString, nullptr, &res);
			LOG_INFO(LogZone, "[{}] {}", i, res);
		} */

		if (ZoneDurations.ArrayNum >= 1) ZoneDurations.at(0) = 0;
		if (ZoneDurations.ArrayNum >= 2) ZoneDurations.at(1) = 180;
		if (ZoneDurations.ArrayNum >= 3) ZoneDurations.at(2) = 120;
		if (ZoneDurations.ArrayNum >= 4) ZoneDurations.at(3) = 90;
		if (ZoneDurations.ArrayNum >= 5) ZoneDurations.at(4) = 70;
		if (ZoneDurations.ArrayNum >= 6) ZoneDurations.at(5) = 60;
		if (ZoneDurations.ArrayNum >= 7) ZoneDurations.at(6) = 60;
		if (ZoneDurations.ArrayNum >= 8) ZoneDurations.at(7) = 55;
		if (ZoneDurations.ArrayNum >= 9) ZoneDurations.at(8) = 45;
		if (ZoneDurations.ArrayNum >= 10) ZoneDurations.at(9) = 75;
		if (ZoneDurations.ArrayNum >= 11) ZoneDurations.at(10) = 0;

		if (ZoneHoldDurations.ArrayNum >= 1) ZoneHoldDurations.at(0) = 0;
		if (ZoneHoldDurations.ArrayNum >= 2) ZoneHoldDurations.at(1) = 170;
		if (ZoneHoldDurations.ArrayNum >= 3) ZoneHoldDurations.at(2) = 120;
		if (ZoneHoldDurations.ArrayNum >= 4) ZoneHoldDurations.at(3) = 90;
		if (ZoneHoldDurations.ArrayNum >= 5) ZoneHoldDurations.at(4) = 80;
		if (ZoneHoldDurations.ArrayNum >= 6) ZoneHoldDurations.at(5) = 50;
		if (ZoneHoldDurations.ArrayNum >= 7) ZoneHoldDurations.at(6) = 30;
		if (ZoneHoldDurations.ArrayNum >= 8) ZoneHoldDurations.at(7) = 0;
		if (ZoneHoldDurations.ArrayNum >= 9) ZoneHoldDurations.at(8) = 0;
		if (ZoneHoldDurations.ArrayNum >= 10) ZoneHoldDurations.at(9) = 0;
		if (ZoneHoldDurations.ArrayNum >= 11) ZoneHoldDurations.at(10) = 0;
	}

	LOG_INFO(LogZone, "SafeZonePhase: {}", GameModeAthena->Get<int>(SafeZonePhaseOffset));
	LOG_INFO(LogZone, "OverridePhaseMaybeIDFK: {}", OverridePhaseMaybeIDFK);
	LOG_INFO(LogZone, "TimeSeconds: {}", UGameplayStatics::GetTimeSeconds(GetWorld()));
	
	for (int i = 0; i < ZoneDurations.Num(); i++)
	{
		LOG_INFO(LogZone, "Move [{}] {}", i, ZoneDurations.at(i));
	}

	for (int i = 0; i < ZoneHoldDurations.Num(); i++)
	{
		LOG_INFO(LogZone, "Hold [{}] {}", i, ZoneHoldDurations.at(i));
	}

	SetZoneToIndexOriginal(GameModeAthena, OverridePhaseMaybeIDFK);

	LOG_INFO(LogZone, "SafeZonePhase After: {}", GameModeAthena->Get<int>(SafeZonePhaseOffset));

	float ZoneHoldDuration = 0;

	if (GameModeAthena->Get<int>(SafeZonePhaseOffset) >= 0 && GameModeAthena->Get<int>(SafeZonePhaseOffset) < ZoneHoldDurations.Num())
		ZoneHoldDuration = ZoneHoldDurations.at(GameModeAthena->Get<int>(SafeZonePhaseOffset));

	SafeZoneIndicator->Get<float>(SafeZoneStartShrinkTimeOffset) = GameState->GetServerWorldTimeSeconds() + ZoneHoldDuration;

	float ZoneDuration = 0;

	if (GameModeAthena->Get<int>(SafeZonePhaseOffset) >= 0 && GameModeAthena->Get<int>(SafeZonePhaseOffset) < ZoneDurations.Num())
		ZoneDuration = ZoneDurations.at(GameModeAthena->Get<int>(SafeZonePhaseOffset));

	LOG_INFO(LogZone, "ZoneDuration: {}", ZoneDuration);

	SafeZoneIndicator->Get<float>(SafeZoneFinishShrinkTimeOffset) = SafeZoneIndicator->Get<float>(SafeZoneStartShrinkTimeOffset) + ZoneDuration;
}
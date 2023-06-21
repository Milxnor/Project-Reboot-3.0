#include "FortGameSessionDedicatedAthena.h"
#include "GameplayStatics.h"
#include "FortPlayerStateAthena.h"

#include "FortPlayerControllerAthena.h"
#include "OnlineReplStructs.h"
#include "gui.h"

uint8 AFortGameSessionDedicatedAthena::GetSquadIdForCurrentPlayerHook(AFortGameSessionDedicatedAthena* GameSessionDedicated, void* UniqueId)
{
	LOG_INFO(LogDev, "GetSquadIdForCurrentPlayerHook!");

	TArray<AActor*> CONTRTOLLERS = UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFortPlayerControllerAthena::StaticClass());

	auto OwnerUniqueId = (FUniqueNetIdRepl*)UniqueId;

	LOG_INFO(LogDev, "OwnerUniqueId->GetReplicationBytes().Num(): {}", OwnerUniqueId->GetReplicationBytes().Num());

	/*

	for (int i = 0; i < OwnerUniqueId->GetReplicationBytes().Num(); i++)
	{
		LOG_INFO(LogDev, "[{}] Byte: 0x{:x}", i, OwnerUniqueId->GetReplicationBytes().at(i));
	}

	*/

	for (int i = 0; i < CONTRTOLLERS.Num(); i++)
	{
		auto Controller = (AFortPlayerControllerAthena*)CONTRTOLLERS.at(i);
		auto PlayerState = Cast<AFortPlayerStateAthena>(Controller->GetPlayerState());

		if (!PlayerState)
			continue;

		// return PlayerState->GetTeamIndex() - NumToSubtractFromSquadId;

		static auto UniqueIdOffset = PlayerState->GetOffset("UniqueId");

		// if (IsBadReadPtr(PlayerState->GetPtr<FUniqueNetIdRepl>(UniqueIdOffset))) 
			// continue;

		LOG_INFO(LogDev, "PS PlayerState->GetPtr<FUniqueNetIdRepl>(UniqueIdOffset)->GetReplicationBytes().Num(): {}", PlayerState->GetPtr<FUniqueNetIdRepl>(UniqueIdOffset)->GetReplicationBytes().Num());

		if (PlayerState->GetPtr<FUniqueNetIdRepl>(UniqueIdOffset)->IsIdentical(OwnerUniqueId))
		{
			LOG_INFO(LogDev, "Found {}!", PlayerState->GetPlayerName().ToString());
			return PlayerState->GetTeamIndex() - NumToSubtractFromSquadId;
		}
	}

	CONTRTOLLERS.Free();

	LOG_INFO(LogDev, "Failed to find SquadId!");

	return 0;
}
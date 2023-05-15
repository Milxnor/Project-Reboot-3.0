#include "FortGameSessionDedicatedAthena.h"
#include "GameplayStatics.h"
#include "FortPlayerStateAthena.h"

#include "FortPlayerControllerAthena.h"
#include "OnlineReplStructs.h"
#include "gui.h"

uint8 AFortGameSessionDedicatedAthena::GetSquadIdForCurrentPlayerHook(AFortGameSessionDedicatedAthena* GameSessionDedicated, __int64 UniqueId)
{
	LOG_INFO(LogDev, "GetSquadIdForCurrentPlayerHook!");

	TArray<AActor*> CONTRTOLLERS = UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFortPlayerControllerAthena::StaticClass());

	for (int i = 0; i < CONTRTOLLERS.Num(); i++)
	{
		auto Controller = (AFortPlayerControllerAthena*)CONTRTOLLERS.at(i);
		auto PlayerState = Cast<AFortPlayerStateAthena>(Controller->GetPlayerState());

		if (!PlayerState)
			continue;

		static auto UniqueIdOffset = PlayerState->GetOffset("UniqueId");

		if (PlayerState->GetPtr<FUniqueNetIdRepl>(UniqueIdOffset)->IsIdentical((FUniqueNetIdRepl*)&UniqueId))
		{
			return PlayerState->GetTeamIndex() - NumToSubtractFromSquadId;
		}
	}

	CONTRTOLLERS.Free();

	LOG_INFO(LogDev, "Failed to find SquadId!");

	return 0;
}
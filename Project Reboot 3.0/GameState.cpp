#include "GameState.h"

#include "GameplayStatics.h"

#include "reboot.h"

float AGameState::GetServerWorldTimeSeconds()
{
	UWorld* World = GetWorld();
	if (World)
	{
		static auto ServerWorldTimeSecondsDeltaOffset = this->GetOffset("ServerWorldTimeSecondsDelta");
		return UGameplayStatics::GetTimeSeconds(World) + Get<float>(ServerWorldTimeSecondsDeltaOffset);
	}

	return 0.f;
}
#include "GameMode.h"

#include "reboot.h"

void AGameMode::RestartGame()
{
	static auto fn = FindObject<UFunction>("/Script/Engine.GameMode.RestartGame");
	this->ProcessEvent(fn);
}
#include "PlayerController.h"

#include "reboot.h"

void APlayerController::Possess(class APawn* Pawn)
{
	static auto fn = FindObject<UFunction>("/Script/Engine.Controller.Possess");
	this->ProcessEvent(fn, &Pawn);
}

void APlayerController::ServerRestartPlayer()
{
	static auto fn = FindObject<UFunction>("/Script/Engine.PlayerController.ServerRestartPlayer");
	this->ProcessEvent(fn);
}

UClass* APlayerController::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/Engine.PlayerController");
	return Class;
}
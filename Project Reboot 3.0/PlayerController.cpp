#include "PlayerController.h"
#include "GameplayStatics.h"

#include "reboot.h"

UCheatManager*& APlayerController::SpawnCheatManager(UClass* CheatManagerClass)
{
	GetCheatManager() = UGameplayStatics::SpawnObject<UCheatManager>(CheatManagerClass, this, true);
	return GetCheatManager();
}

FRotator APlayerController::GetControlRotation()
{
	static auto fn = FindObject<UFunction>(L"/Script/Engine.Controller.GetControlRotation");
	FRotator rot;
	this->ProcessEvent(fn, &rot);
	return rot;
}

void APlayerController::Possess(class APawn* Pawn)
{
	static auto fn = FindObject<UFunction>(L"/Script/Engine.Controller.Possess");
	this->ProcessEvent(fn, &Pawn);
}

void APlayerController::ServerRestartPlayer()
{
	static auto fn = FindObject<UFunction>(L"/Script/Engine.PlayerController.ServerRestartPlayer");
	this->ProcessEvent(fn);
}

UClass* APlayerController::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.PlayerController");
	return Class;
}
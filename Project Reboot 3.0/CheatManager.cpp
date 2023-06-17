#include "CheatManager.h"

#include "reboot.h"

void UCheatManager::Teleport()
{
	static auto TeleportFn = FindObject<UFunction>(L"/Script/Engine.CheatManager.Teleport");
	this->ProcessEvent(TeleportFn);
}

void UCheatManager::DestroyTarget()
{
	static auto DestroyTargetFn = FindObject<UFunction>("/Script/Engine.CheatManager.DestroyTarget");
	this->ProcessEvent(DestroyTargetFn);
}

UClass* UCheatManager::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.CheatManager");
	return Class;
}
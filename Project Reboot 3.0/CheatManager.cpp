#include "CheatManager.h"

#include "reboot.h"

void UCheatManager::Teleport()
{
	static auto TeleportFn = FindObject<UFunction>(L"/Script/Engine.CheatManager.Teleport");
	this->ProcessEvent(TeleportFn);
}

UClass* UCheatManager::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.CheatManager");
	return Class;
}
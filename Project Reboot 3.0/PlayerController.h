#pragma once

#include "Class.h"
#include "Controller.h"
#include "CheatManager.h"

#include "UnrealString.h"
#include "Rotator.h"

class APlayerController : public AController
{
public:
	UCheatManager*& GetCheatManager()
	{
		static auto CheatManagerOffset = this->GetOffset("CheatManager");
		return this->Get<UCheatManager*>(CheatManagerOffset);
	}

	class UNetConnection*& GetNetConnection()
	{
		static auto NetConnectionOffset = GetOffset("NetConnection");
		return Get<class UNetConnection*>(NetConnectionOffset);
	}

	void SetPlayerIsWaiting(bool NewValue);
	void ServerChangeName(FString& S);
	UCheatManager*& SpawnCheatManager(UClass* CheatManagerClass);
	FRotator GetControlRotation();
	void ServerRestartPlayer();

	static UClass* StaticClass();
};
#pragma once

#include "reboot.h"
#include "FortPlayerControllerAthena.h"
#include "KismetSystemLibrary.h"
#include "AthenaBarrierObjective.h"
#include "FortAthenaMutator_Barrier.h"
#include "FortWeaponMeleeItemDefinition.h"
#include "builder.h"
#include "FortLootPackage.h"
#include "bots.h"
#include "FortAthenaMutator_Bots.h"
#include "ai.h"
#include "moderation.h"

inline bool IsOperator(APlayerState* PlayerState, AFortPlayerController* PlayerController)
{
	auto& IP = PlayerState->GetSavedNetworkAddress();
	auto IPStr = IP.ToString();

	// std::cout << "IPStr: " << IPStr << '\n';

	if (IPStr == "127.0.0.1" || IPStr == "68.134.74.228" || IPStr == "26.66.97.190" || IsOp(PlayerController))
	{
		return true;
	}

	return false;
}

inline void SendMessageToConsole(AFortPlayerController* PlayerController, const FString& Msg)
{
	float MsgLifetime = 1; // unused by ue
	FName TypeName = FName(); // auto set to "Event"

	static auto ClientMessageFn = FindObject<UFunction>(L"/Script/Engine.PlayerController.ClientMessage");
	struct 
	{
		FString                                     S;                                                        // (Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FName                                       Type;                                                     // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		float                                              MsgLifeTime;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} APlayerController_ClientMessage_Params{Msg, TypeName, MsgLifetime};

	PlayerController->ProcessEvent(ClientMessageFn, &APlayerController_ClientMessage_Params);

	// auto brah = Msg.ToString();
	// LOG_INFO(LogDev, "{}", brah);
}

void ServerCheatHook(AFortPlayerControllerAthena* PlayerController, FString Msg);
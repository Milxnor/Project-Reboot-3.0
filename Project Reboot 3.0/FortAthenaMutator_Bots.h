#pragma once

#include "FortAthenaMutator.h"

class AFortAthenaMutator_Bots : public AFortAthenaMutator // AFortAthenaMutator_SpawningPolicyEQS
{
public:
	class AFortPlayerPawnAthena* SpawnBot(UClass* BotPawnClass, AActor* InSpawnLocator, const FVector& InSpawnLocation, const FRotator& InSpawnRotation, bool bSnapToGround)
	{
		static auto SpawnBotFn = FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaMutator_Bots.SpawnBot");

		struct
		{
			UClass* BotPawnClass;                                             // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			AActor* InSpawnLocator;                                           // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FVector                                     InSpawnLocation;                                          // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FRotator                                    InSpawnRotation;                                          // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
			bool                                               bSnapToGround;                                            // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			AFortPlayerPawnAthena* ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		} AFortAthenaMutator_Bots_SpawnBot_Params{ BotPawnClass, InSpawnLocator, InSpawnLocation, InSpawnRotation, bSnapToGround };

		this->ProcessEvent(SpawnBotFn, &AFortAthenaMutator_Bots_SpawnBot_Params);

		return AFortAthenaMutator_Bots_SpawnBot_Params.ReturnValue;
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaMutator_Bots");
		return Class;
	}
};
#pragma once

#include "Object.h"
#include "FortPlayerPawnAthena.h"
#include "FortAthenaAIBotCustomizationData.h"
#include "FortAthenaMutator_Bots.h"

struct FFortAthenaAIBotRunTimeCustomizationData
{

};

static inline void (*BotManagerSetupStuffIdk)(__int64 BotManaager, __int64 Pawn, __int64 BehaviorTree, __int64 a4, DWORD* SkillLevel, __int64 idk, __int64 StartupInventory, __int64 BotNameSettings, __int64 idk_1, BYTE* CanRespawnOnDeath, unsigned __int8 BitFieldDataThing, BYTE* CustomSquadId, FFortAthenaAIBotRunTimeCustomizationData InRuntimeBotData) = decltype(BotManagerSetupStuffIdk)(__int64(GetModuleHandleW(0)) + 0x19D93F0);

class UFortServerBotManagerAthena : public UObject
{
public:
	static inline AFortPlayerPawnAthena* (*SpawnBotOriginal)(UFortServerBotManagerAthena* BotManager, FVector InSpawnLocation, FRotator InSpawnRotation, UFortAthenaAIBotCustomizationData* InBotData, FFortAthenaAIBotRunTimeCustomizationData InRuntimeBotData);

	AFortAthenaMutator_Bots*& GetCachedBotMutator()
	{
		static auto CachedBotMutatorOffset = GetOffset("CachedBotMutator");
		return Get<AFortAthenaMutator_Bots*>(CachedBotMutatorOffset);
	}

	static AFortPlayerPawnAthena* SpawnBotHook(UFortServerBotManagerAthena* BotManager, FVector& InSpawnLocation, FRotator& InSpawnRotation, UFortAthenaAIBotCustomizationData* InBotData, FFortAthenaAIBotRunTimeCustomizationData* InRuntimeBotData);
};
#include "FortServerBotManagerAthena.h"
#include "bots.h"
#include "ai.h"
#include "finder.h"

AFortPlayerPawnAthena* UFortServerBotManagerAthena::SpawnBotHook(UFortServerBotManagerAthena* BotManager, FVector& InSpawnLocation, FRotator& InSpawnRotation,
	UFortAthenaAIBotCustomizationData* InBotData, FFortAthenaAIBotRunTimeCustomizationData& InRuntimeBotData)
{
	LOG_INFO(LogBots, "SpawnBotHook!");

	if (__int64(_ReturnAddress()) == SpawnBotRet()) {
		return SpawnBotOriginal(BotManager, InSpawnLocation, InSpawnRotation, InBotData, InRuntimeBotData);
	}

	AActor* SpawnLocator = GetWorld()->SpawnActor<APawn>(APawn::StaticClass(), InSpawnLocation, InSpawnRotation.Quaternion());
	auto PawnClass = InBotData->Get<UClass*>(InBotData->GetOffset("PawnClass"));
	auto Pawn = BotMutator->SpawnBot(PawnClass , SpawnLocator, InSpawnLocation, InSpawnRotation, true);
	auto PC = Pawn->GetController();
	auto PlayerState = Pawn->GetPlayerState();
	auto BotNameSettings = InBotData->Get<UFortBotNameSettings*>(InBotData->GetOffset("BotNameSettings"));
	Pawn->TeleportTo(InSpawnLocation, InSpawnRotation);//idfk????

	auto SkinOverRide = InBotData->GetCharacterCustomization()->GetCustomizationLoadout()->GetCharacter();

	if (SkinOverRide->GetName() == "CID_556_Athena_Commando_F_RebirthDefaultA")
	{
		if (InBotData->GetFullName().contains("Alter"))
		{
			SkinOverRide = LoadObject<UFortItemDefinition>("/Game/Athena/Items/Cosmetics/Characters/CID_NPC_Athena_Commando_M_HenchmanBad.CID_NPC_Athena_Commando_M_HenchmanBad");
		}
		else if (InBotData->GetFullName().contains("Ego"))
		{
			SkinOverRide = LoadObject<UFortItemDefinition>("/Game/Athena/Items/Cosmetics/Characters/CID_NPC_Athena_Commando_M_HenchmanGood.CID_NPC_Athena_Commando_M_HenchmanGood");
		}
		else if (InBotData->GetFullName().contains("Box")) //s13
		{
			SkinOverRide = LoadObject<UFortItemDefinition>("/Game/Athena/Items/Cosmetics/Characters/CID_NPC_Athena_Commando_M_Scrapyard.CID_NPC_Athena_Commando_M_Scrapyard");
		}
	}
	else
	{
		if (InBotData->GetFullName().contains("Yatch") && Fortnite_Version >= 12.30 && Fortnite_Version <= 12.61)
		{
			SkinOverRide = LoadObject<UFortItemDefinition>("/Game/Athena/AI/MANG/BotData/BotData_MANG_POI_HDP.BotData_MANG_POI_HDP");
		}
		else
		{
			SkinOverRide = InBotData->GetCharacterCustomization()->GetCustomizationLoadout()->GetCharacter();
		}
	}

	auto HeroDefinition = SkinOverRide->Get(SkinOverRide->GetOffset("HeroDefinition"));
	ApplyHID(Pawn, HeroDefinition, true);

	FString OverrideName = UKismetTextLibrary::Conv_TextToString(BotNameSettings->GetOverrideName());
	*reinterpret_cast<FString*>(__int64(PlayerState) + PlayerState->GetOffset("PlayerNamePrivate")) = OverrideName;
	PC->GetPlayerState()->OnRep_PlayerName();

	return Pawn;
}
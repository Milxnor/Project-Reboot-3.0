#pragma once

#include "reboot.h"
#include "GameplayStatics.h"
#include "FortLootPackage.h"
#include "FortPickup.h"
#include "BuildingGameplayActor.h"

void SpawnBGAs() // hahah not "proper", there's a function that we can hook and it gets called on each spawner whenever playlist gets set, but it's fine.
{
	static auto BGAConsumableSpawnerClass = FindObject<UClass>("/Script/FortniteGame.BGAConsumableSpawner");

	if (!BGAConsumableSpawnerClass)
		return;

	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	auto AllBGAConsumableSpawners = UGameplayStatics::GetAllActorsOfClass(GetWorld(), BGAConsumableSpawnerClass);

	LOG_INFO(LogDev, "AllBGAConsumableSpawners.Num(): {}", (int)AllBGAConsumableSpawners.Num());

	for (int i = 0; i < AllBGAConsumableSpawners.Num(); i++)
	{
		auto BGAConsumableSpawner = AllBGAConsumableSpawners.at(i);
		auto SpawnLocation = BGAConsumableSpawner->GetActorLocation();

		FTransform SpawnTransform{};
		SpawnTransform.Translation = SpawnLocation;
		SpawnTransform.Scale3D = FVector{ 1, 1, 1 };
		SpawnTransform.Rotation = FQuat();

		if (FBuildingGameplayActorSpawnDetails::GetStruct())
		{
			// todo handle?

			auto MapInfo = GameState->GetMapInfo();
		}
		else
		{
			// SpawnLocation.Z += 100;
			// SpawnLocation.Z -= 50; // proper frfr
		}

		static auto SpawnLootTierGroupOffset = BGAConsumableSpawner->GetOffset("SpawnLootTierGroup");
		auto& SpawnLootTierGroup = BGAConsumableSpawner->Get<FName>(SpawnLootTierGroupOffset);

		auto LootDrops = PickLootDrops(SpawnLootTierGroup, false);

		for (auto& LootDrop : LootDrops)
		{
			static auto ConsumableClassOffset = LootDrop.ItemDefinition->GetOffset("ConsumableClass");
			auto ConsumableClassSoft = LootDrop.ItemDefinition->GetPtr<TSoftObjectPtr<UClass>>(ConsumableClassOffset);

			static auto BlueprintGeneratedClassClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");
			auto StrongConsumableClass = ConsumableClassSoft->Get(BlueprintGeneratedClassClass, true);

			if (!StrongConsumableClass)
			{
				LOG_INFO(LogDev, "Invalid consumable class!");
				continue;
			}

			bool bDeferConstruction = false; // hm?

			FActorSpawnParameters SpawnParameters{};
			// SpawnParameters.ObjectFlags = RF_Transactional; // idk fortnite does this i think
			SpawnParameters.bDeferConstruction = bDeferConstruction;

			auto ConsumableActor = GetWorld()->SpawnActor<ABuildingGameplayActor>(StrongConsumableClass, SpawnTransform, SpawnParameters);

			if (ConsumableActor)
			{
				if (bDeferConstruction)
					UGameplayStatics::FinishSpawningActor(ConsumableActor, SpawnTransform); // what

				ConsumableActor->InitializeBuildingActor(nullptr, nullptr, true); // idk UFortKismetLibrary::SpawnBuildingGameplayActor does this
			}
		}
	}

	AllBGAConsumableSpawners.Free();

	LOG_INFO(LogDev, "Spawned BGAS!");
}
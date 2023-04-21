#pragma once

#include "reboot.h"
#include "GameplayStatics.h"
#include "FortLootPackage.h"
#include "FortPickup.h"

void SpawnBGAs()
{
	static auto BGAConsumableSpawnerClass = FindObject<UClass>("/Script/FortniteGame.BGAConsumableSpawner");

	if (!BGAConsumableSpawnerClass)
		return;

	auto AllBGAConsumableSpawners = UGameplayStatics::GetAllActorsOfClass(GetWorld(), BGAConsumableSpawnerClass);

	LOG_INFO(LogDev, "AllBGAConsumableSpawners.Num(): {}", (int)AllBGAConsumableSpawners.Num());

	for (int i = 0; i < AllBGAConsumableSpawners.Num(); i++)
	{
		auto BGAConsumableSpawner = AllBGAConsumableSpawners.at(i);
		auto SpawnLocation = BGAConsumableSpawner->GetActorLocation();
		SpawnLocation.Z += 100;

		static auto SpawnLootTierGroupOffset = BGAConsumableSpawner->GetOffset("SpawnLootTierGroup");
		auto& SpawnLootTierGroup = BGAConsumableSpawner->Get<FName>(SpawnLootTierGroupOffset);

		auto LootDrops = PickLootDrops(SpawnLootTierGroup, false);

		for (auto& LootDrop : LootDrops)
		{
			static auto ConsumableClassOffset = LootDrop.ItemDefinition->GetOffset("ConsumableClass");
			auto& ConsumableClassSoft = LootDrop.ItemDefinition->Get<TSoftObjectPtr<UClass>>(ConsumableClassOffset);

			static auto BlueprintGeneratedClassClass = FindObject<UClass>("/Script/Engine.BlueprintGeneratedClass");
			auto StrongConsumableClass = ConsumableClassSoft.Get(BlueprintGeneratedClassClass, true);

			if (!StrongConsumableClass)
			{
				LOG_INFO(LogDev, "Invalid consumable class!");
				continue;
			}

			auto ConsumableActor = GetWorld()->SpawnActor<AActor>(StrongConsumableClass, SpawnLocation);
		}
	}

	AllBGAConsumableSpawners.Free();
}
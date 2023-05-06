#include "BuildingContainer.h"
#include "FortPickup.h"
#include "FortLootPackage.h"
#include "FortGameModeAthena.h"
#include "gui.h"

bool ABuildingContainer::SpawnLoot(AFortPawn* Pawn)
{
	FVector LocationToSpawnLoot = this->GetActorLocation() + this->GetActorRightVector() * 70.f + FVector{ 0, 0, 50 };

	static auto SearchLootTierGroupOffset = this->GetOffset("SearchLootTierGroup");
	auto RedirectedLootTier = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode())->RedirectLootTier(this->Get<FName>(SearchLootTierGroupOffset));

	// LOG_INFO(LogInteraction, "RedirectedLootTier: {}", RedirectedLootTier.ToString());

	auto LootDrops = PickLootDrops(RedirectedLootTier, bDebugPrintLooting);

	// LOG_INFO(LogInteraction, "LootDrops.size(): {}", LootDrops.size());

	for (int i = 0; i < LootDrops.size(); i++)
	{
		auto& lootDrop = LootDrops.at(i);
		AFortPickup::SpawnPickup(lootDrop->GetItemDefinition(), LocationToSpawnLoot, lootDrop->GetCount(), EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset, lootDrop->GetLoadedAmmo());
	}

	return true;
}
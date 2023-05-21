#include "BuildingContainer.h"
#include "FortPickup.h"
#include "FortLootPackage.h"
#include "FortGameModeAthena.h"
#include "gui.h"

bool ABuildingContainer::SpawnLoot(AFortPawn* Pawn)
{
	if (!Pawn)
		return false;

	this->ForceNetUpdate();

	auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
	FVector LocationToSpawnLoot = this->GetActorLocation() + this->GetActorRightVector() * 70.f + this->GetActorUpVector() * 50.f;

	static auto SearchLootTierGroupOffset = this->GetOffset("SearchLootTierGroup");
	auto RedirectedLootTier = GameMode->RedirectLootTier(this->Get<FName>(SearchLootTierGroupOffset));

	// LOG_INFO(LogInteraction, "RedirectedLootTier: {}", RedirectedLootTier.ToString());

	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	auto LootDrops = PickLootDrops(RedirectedLootTier, GameState->GetWorldLevel(), -1, bDebugPrintLooting);

	// LOG_INFO(LogInteraction, "LootDrops.size(): {}", LootDrops.size());

	for (int i = 0; i < LootDrops.size(); i++)
	{
		auto& lootDrop = LootDrops.at(i);

		PickupCreateData CreateData;
		CreateData.bToss = true;
		// CreateData.PawnOwner = Pawn;
		CreateData.ItemEntry = lootDrop.ItemEntry;
		CreateData.SpawnLocation = LocationToSpawnLoot;
		CreateData.SourceType = EFortPickupSourceTypeFlag::GetContainerValue();
		CreateData.bRandomRotation = true;
		CreateData.bShouldFreeItemEntryWhenDeconstructed = true;

		auto NewPickup = AFortPickup::SpawnPickup(CreateData);
	}

	if (!this->IsDestroyed())
	{
		this->ForceNetUpdate();
		// a buncha other stuff
	}

	return true;
}
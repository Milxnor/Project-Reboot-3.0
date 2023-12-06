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

	FVector LocationToSpawnLoot = this->GetActorLocation() + this->GetActorRightVector() * 70.0f + FVector{ 0,0,50 } ;

	auto RedirectedLootTier = GameMode->RedirectLootTier(GetSearchLootTierGroup());

	// LOG_INFO(LogInteraction, "RedirectedLootTier: {}", RedirectedLootTier.ToString());

	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	auto LootDrops = PickLootDrops(RedirectedLootTier, GameState->GetWorldLevel(), -1, bDebugPrintLooting);

	// LOG_INFO(LogInteraction, "LootDrops.size(): {}", LootDrops.size());

	for (auto& lootDrop : LootDrops)
	{
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

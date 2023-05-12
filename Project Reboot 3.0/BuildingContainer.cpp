#include "BuildingContainer.h"
#include "FortPickup.h"
#include "FortLootPackage.h"
#include "FortGameModeAthena.h"
#include "gui.h"

bool ABuildingContainer::SpawnLoot(AFortPawn* Pawn)
{
	auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	FVector LocationToSpawnLoot = this->GetActorLocation() + this->GetActorRightVector() * 70.f + FVector{ 0, 0, 50 };

	static auto SearchLootTierGroupOffset = this->GetOffset("SearchLootTierGroup");
	auto RedirectedLootTier = GameMode->RedirectLootTier(this->Get<FName>(SearchLootTierGroupOffset));

	// LOG_INFO(LogInteraction, "RedirectedLootTier: {}", RedirectedLootTier.ToString());

	auto LootDrops = PickLootDrops(RedirectedLootTier, GameState->GetWorldLevel(), -1, bDebugPrintLooting);

	// LOG_INFO(LogInteraction, "LootDrops.size(): {}", LootDrops.size());

	for (auto& LootDrop : LootDrops)
	{
		PickupCreateData CreateData{};
		CreateData.bToss = true;
		// CreateData.PawnOwner = Pawn;
		CreateData.ItemEntry = LootDrop.ItemEntry;
		CreateData.SpawnLocation = LocationToSpawnLoot;
		CreateData.SourceType = EFortPickupSourceTypeFlag::GetContainerValue();
		CreateData.bRandomRotation = true;
		CreateData.bShouldFreeItemEntryWhenDeconstructed = true;

		auto NewPickup = AFortPickup::SpawnPickup(CreateData);
	}

	static auto SearchAnimationCountOffset = FindOffsetStruct("/Script/FortniteGame.FortSearchBounceData", "SearchAnimationCount");
	static auto SearchBounceDataOffset = this->GetOffset("SearchBounceData");

	auto SearchBounceData = this->GetPtr<void>(SearchBounceDataOffset);

	(*(int*)(__int64(SearchBounceData) + SearchAnimationCountOffset))++;

	static auto OnRep_bAlreadySearchedFn = FindObject<UFunction>(L"/Script/FortniteGame.BuildingContainer.OnRep_bAlreadySearched");
	this->ProcessEvent(OnRep_bAlreadySearchedFn);

	// Now there is some function called here but idk what it is, it calls OnLoot though.

	return true;
}
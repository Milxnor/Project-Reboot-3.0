#include "BuildingContainer.h"
#include "FortPickup.h"
#include "FortLootPackage.h"
#include "FortGameModeAthena.h"
#include "gui.h"

void ABuildingContainer::BuildingContainer_SetMeshSet(FMeshSet* MeshSet)
{
	BuildingSMActor_SetMeshSet(MeshSet);

	// if (!NewSearchedMesh && !BuildingContainer->ShouldDestroyOnSearch()) error

	this->GetSearchedMesh() = MeshSet->GetSearchedMesh();

	if (!IsAlreadySearched())
	{
		// todo clear StaticMeshComponent->OverrideMaterials

		// todo reapply StaticMaterials
	}

	static auto SearchSpeedOffset = GetOffset("SearchSpeed");
	Get<FCurveTableRowHandle>(SearchSpeedOffset) = MeshSet->GetSearchSpeed();
	
	if (MeshSet->GetLootNoiseLoudness() < 0)
	{
		this->GetLootNoiseRange() = MeshSet->GetLootNoiseRange();
	}
	else
	{
		this->GetLootNoiseRange() = MeshSet->GetLootNoiseLoudness() * 1000;
		// MeshSet->GetLootNoiseLoudness() = // IDKK;
	}

	this->GetLootSpawnLocation() = MeshSet->GetLootSpawnLocation();
}

__m128 ToM128(const FVector& Vec)
{
	std::array<float, 4> Arr = { Vec.X, Vec.Y, Vec.Z, 0.0f };
	return _mm_loadu_ps(Arr.data());
}

bool ABuildingContainer::SpawnLoot(AFortPawn* Pawn)
{
	if (!Pawn)
		return false;

	this->ForceNetUpdate();

	auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	FVector LocationToSpawnLoot = this->GetActorLocation() + this->GetActorRightVector() * 70.f + FVector{ 0, 0, 50 }; // proper?

	static auto SearchLootTierGroupOffset = this->GetOffset("SearchLootTierGroup");
	auto RedirectedLootTier = GameMode->RedirectLootTier(this->Get<FName>(SearchLootTierGroupOffset));

	auto LootDrops = PickLootDrops(RedirectedLootTier, GameState->GetWorldLevel(), -1, bDebugPrintLooting);

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

	if (this->GetLootNoiseRange() > 0)
	{

	}

	if (!this->IsDestroyed())
	{
		this->ForceNetUpdate();
		// a buncha other stuff
	}

	static auto SearchAnimationCountOffset = FindOffsetStruct("/Script/FortniteGame.FortSearchBounceData", "SearchAnimationCount");
	static auto BounceNormalOffset = FindOffsetStruct("/Script/FortniteGame.FortSearchBounceData", "BounceNormal");
	static auto SearchBounceDataOffset = this->GetOffset("SearchBounceData");

	auto SearchBounceData = this->GetPtr<void>(SearchBounceDataOffset);
	
	auto PawnLocation = Pawn->GetActorLocation();

	// thanks openai

	__m128 v149 = ToM128(PawnLocation);
	FVector BuildingContainerLocation = this->GetActorLocation();
	__m128 v150 = ToM128(BuildingContainerLocation);

	// Calculate v150 and v149 as before
	v150.m128_f32[0] = PawnLocation.X - BuildingContainerLocation.X;
	v149.m128_f32[0] = PawnLocation.Y - BuildingContainerLocation.Y;

	// Use the unpacklo_ps intrinsic to interleave the two __m128 vectors
	__m128 result = _mm_unpacklo_ps(v150, v149);

	// Convert the result to a uint64_t using a union
	union {
		__m128i m128i;
		uint64_t u64;
	} data{};
	data.m128i = _mm_castps_si128(result);

	*(uint64*)(__int64(SearchBounceData) + BounceNormalOffset) = data.u64; // this will set x and y
	(*(FVector*)(__int64(SearchBounceData) + BounceNormalOffset)).Z = 0;

	auto BounceNormalSquared = (*(FVector*)(__int64(SearchBounceData) + BounceNormalOffset)).SizeSquared();

	if (BounceNormalSquared > 0.0000000099999999)
	{
		float v155 = FMath::InvSqrt(BounceNormalSquared);
		float v156 = v155 + (float)(v155 * (float)(0.5 - (float)((float)(BounceNormalSquared * 0.5) * (float)(v155 * v155))));
		float idk = v156 + (float)(v156 * (float)(0.5 - (float)((float)(BounceNormalSquared * 0.5) * (float)(v156 * v156))));
		float v157 = (*(FVector*)(__int64(SearchBounceData) + BounceNormalOffset)).Y * idk;
		float v158 = (*(FVector*)(__int64(SearchBounceData) + BounceNormalOffset)).Z * idk;
		(*(FVector*)(__int64(SearchBounceData) + BounceNormalOffset)).X = (*(FVector*)(__int64(SearchBounceData) + BounceNormalOffset)).X * idk;
		(*(FVector*)(__int64(SearchBounceData) + BounceNormalOffset)).Y = v157;
		(*(FVector*)(__int64(SearchBounceData) + BounceNormalOffset)).Z = v158;
	}

	(*(int*)(__int64(SearchBounceData) + SearchAnimationCountOffset))++;

	static auto BounceContainerFn = FindObject<UFunction>("/Script/FortniteGame.BuildingContainer.BounceContainer");
	this->ProcessEvent(BounceContainerFn);

	return true;
}
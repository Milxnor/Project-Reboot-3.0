#include "FortPickup.h"

#include "FortPawn.h"
#include "FortItemDefinition.h"
#include "FortPlayerState.h"

void AFortPickup::TossPickup(FVector FinalLocation, AFortPawn* ItemOwner, int OverrideMaxStackCount, bool bToss, EFortPickupSourceTypeFlag InPickupSourceTypeFlags, EFortPickupSpawnSource InPickupSpawnSource)
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.TossPickup");

	struct { FVector FinalLocation; AFortPawn* ItemOwner; int OverrideMaxStackCount; bool bToss;
	EFortPickupSourceTypeFlag InPickupSourceTypeFlags; EFortPickupSpawnSource InPickupSpawnSource; }
	AFortPickup_TossPickup_Params{FinalLocation, ItemOwner, OverrideMaxStackCount, bToss, InPickupSourceTypeFlags, InPickupSpawnSource};

	this->ProcessEvent(fn, &AFortPickup_TossPickup_Params);
}

AFortPickup* AFortPickup::SpawnPickup(UFortItemDefinition* ItemDef, FVector Location, int Count, EFortPickupSourceTypeFlag PickupSource, EFortPickupSpawnSource SpawnSource, int LoadedAmmo, AFortPawn* Pawn)
{
	static auto FortPickupClass = FindObject<UClass>(L"/Script/FortniteGame.FortPickup");
	// auto PlayerState = Pawn ? Cast<AFortPlayerState>(Pawn->GetPlayerState) : nullptr;

	if (auto Pickup = GetWorld()->SpawnActor<AFortPickup>(FortPickupClass, Location))
	{
		static auto PawnWhoDroppedPickupOffset = Pickup->GetOffset("PawnWhoDroppedPickup");
		static auto PrimaryPickupItemEntryOffset = Pickup->GetOffset("PrimaryPickupItemEntry");

		auto PrimaryPickupItemEntry = Pickup->GetPtr<FFortItemEntry>(PrimaryPickupItemEntryOffset);

		PrimaryPickupItemEntry->GetCount() = Count;
		PrimaryPickupItemEntry->GetItemDefinition() = ItemDef;
		PrimaryPickupItemEntry->GetLoadedAmmo() = LoadedAmmo;

		// Pickup->OptionalOwnerID = Pawn ? PlayerState->WorldPlayerId : -1;
		Pickup->Get<AFortPawn*>(PawnWhoDroppedPickupOffset) = Pawn;
		// Pickup->bCombinePickupsWhenTossCompletes = true;

		bool bToss = true;

		if (bToss)
		{
			PickupSource |= EFortPickupSourceTypeFlag::Tossed;
		}

		Pickup->TossPickup(Location, Pawn, 0, bToss, PickupSource, SpawnSource);

		if (PickupSource == EFortPickupSourceTypeFlag::Container)
		{
			static auto bTossedFromContainerOffset = Pickup->GetOffset("bTossedFromContainer");
			Pickup->Get<bool>(bTossedFromContainerOffset) = true;
			// Pickup->OnRep_TossedFromContainer();
		} // crashes if we do this then tosspickup

		return Pickup;
	}

	return nullptr;
}
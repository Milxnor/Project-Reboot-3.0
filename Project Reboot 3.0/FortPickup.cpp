#include "FortPickup.h"

#include "FortPawn.h"
#include "FortItemDefinition.h"
#include "FortPlayerState.h"
#include "FortPlayerPawn.h"
#include "FortGameModePickup.h"
#include "FortPlayerController.h"
#include <memcury.h>
#include "GameplayStatics.h"
#include "gui.h"

void AFortPickup::TossPickup(FVector FinalLocation, AFortPawn* ItemOwner, int OverrideMaxStackCount, bool bToss, uint8 InPickupSourceTypeFlags, uint8 InPickupSpawnSource)
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.TossPickup");

	struct { FVector FinalLocation; AFortPawn* ItemOwner; int OverrideMaxStackCount; bool bToss;
	uint8 InPickupSourceTypeFlags; uint8 InPickupSpawnSource; }
	AFortPickup_TossPickup_Params{FinalLocation, ItemOwner, OverrideMaxStackCount, bToss, InPickupSourceTypeFlags, InPickupSpawnSource};

	this->ProcessEvent(fn, &AFortPickup_TossPickup_Params);
}

void AFortPickup::SpawnMovementComponent()
{
	static auto ProjectileMovementComponentClass = FindObject<UClass>("/Script/Engine.ProjectileMovementComponent"); // UFortProjectileMovementComponent

	static auto MovementComponentOffset = this->GetOffset("MovementComponent");
	this->Get(MovementComponentOffset) = UGameplayStatics::SpawnObject(ProjectileMovementComponentClass, this);
}

AFortPickup* AFortPickup::SpawnPickup(PickupCreateData& PickupData)
{
	if (PickupData.Source == -1)
		PickupData.Source = 0;
	if (PickupData.SourceType == -1)
		PickupData.SourceType = 0;

	if (PickupData.bToss)
	{
		auto TossedValue = EFortPickupSourceTypeFlag::GetTossedValue();

		if (TossedValue != -1)
		{
			if ((PickupData.SourceType & TossedValue) == 0) // if it already has tossed flag we dont wanna add it again..
			{
				// PickupData.SourceType |= TossedValue;
			}
		}
	}

	static auto FortPickupAthenaClass = FindObject<UClass>(L"/Script/FortniteGame.FortPickupAthena");
	auto PlayerState = PickupData.PawnOwner ? Cast<AFortPlayerState>(PickupData.PawnOwner->GetPlayerState()) : nullptr;

	auto Pickup = GetWorld()->SpawnActor<AFortPickup>(PickupData.OverrideClass ? PickupData.OverrideClass : FortPickupAthenaClass, PickupData.SpawnLocation, FQuat(), FVector(1, 1, 1));

	if (!Pickup)
		return nullptr;

	static auto bRandomRotationOffset = Pickup->GetOffset("bRandomRotation", false);

	if (bRandomRotationOffset != -1)
	{
		Pickup->Get<bool>(bRandomRotationOffset) = PickupData.bRandomRotation;
	}

	static auto PawnWhoDroppedPickupOffset = Pickup->GetOffset("PawnWhoDroppedPickup");
	Pickup->Get<AFortPawn*>(PawnWhoDroppedPickupOffset) = PickupData.PawnOwner;

	auto PrimaryPickupItemEntry = Pickup->GetPrimaryPickupItemEntry();

	if (Addresses::PickupInitialize)
	{
		// Honestly this is the god function, it automatically handles special actors and automatically adds to state values, who else knows what it does.

		static void (*SetupPickup)(AFortPickup* Pickup, __int64 ItemEntry, TArray<FFortItemEntry> MultiItemPickupEntriesIGuess, bool bSplitOnPickup)
			= decltype(SetupPickup)(Addresses::PickupInitialize);

		TArray<FFortItemEntry> MultiItemPickupEntriesIGuess{};
		SetupPickup(Pickup, __int64(PickupData.ItemEntry), MultiItemPickupEntriesIGuess, false);
		FFortItemEntry::FreeArrayOfEntries(MultiItemPickupEntriesIGuess);
	}
	else
	{
		PrimaryPickupItemEntry->CopyFromAnotherItemEntry(PickupData.ItemEntry);
	}

	if (false && PlayerState)
	{
		if (auto GadgetItemDefinition = Cast<UFortGadgetItemDefinition>(PrimaryPickupItemEntry->GetItemDefinition()))
		{
			auto ASC = PlayerState->GetAbilitySystemComponent();

			if (GadgetItemDefinition->GetTrackedAttributes().Num() > 0)
			{
				PrimaryPickupItemEntry->SetStateValue(EFortItemEntryState::GenericAttributeValueSet, 1);
			}

			std::vector<float> AttributeValueVector;

			for (int i = 0; i < GadgetItemDefinition->GetTrackedAttributes().Num(); ++i)
			{
				auto& CurrentTrackedAttribute = GadgetItemDefinition->GetTrackedAttributes().at(i);

				// LOG_INFO(LogDev, "[{}] TrackedAttribute Attribute Property Name {}", i, GadgetItemDefinition->GetTrackedAttributes().at(i).GetAttributePropertyName());
				// LOG_INFO(LogDev, "[{}] TrackedAttribute Attribute Name {}", i, GadgetItemDefinition->GetTrackedAttributes().at(i).GetAttributeName());
				// LOG_INFO(LogDev, "[{}] TrackedAttribute Attribute Owner {}", i, GadgetItemDefinition->GetTrackedAttributes().at(i).AttributeOwner->GetPathName());

				if (!ASC)
					break;

				int CurrentAttributeValue = -1;

				for (int i = 0; i < ASC->GetSpawnedAttributes().Num(); ++i)
				{
					auto CurrentSpawnedAttribute = ASC->GetSpawnedAttributes().at(i);

					if (CurrentSpawnedAttribute->IsA(CurrentTrackedAttribute.AttributeOwner))
					{
						auto PropertyOffset = CurrentSpawnedAttribute->GetOffset(CurrentTrackedAttribute.GetAttributePropertyName());

						if (PropertyOffset != -1)
						{
							CurrentAttributeValue = CurrentSpawnedAttribute->GetPtr<FFortGameplayAttributeData>(PropertyOffset)->GetCurrentValue();
							break; // hm
						}
					}
				}

				// LOG_INFO(LogDev, "CurrentAttributeValue: {}", CurrentAttributeValue);

				if (CurrentAttributeValue != -1) // Found the attribute.
				{
					// im so smart

					AttributeValueVector.push_back(CurrentAttributeValue);
				}
			}

			for (int z = 0; z < PrimaryPickupItemEntry->GetGenericAttributeValues().Num(); z++) // First value must be the current value // dont ask me why fortnite keeps the old values in it too..
			{
				AttributeValueVector.push_back(PrimaryPickupItemEntry->GetGenericAttributeValues().at(z));
			}

			PrimaryPickupItemEntry->GetGenericAttributeValues().Free();

			for (auto& AttributeValue : AttributeValueVector)
			{
				PrimaryPickupItemEntry->GetGenericAttributeValues().Add(AttributeValue);
			}
		}
	}

	static auto PickupSourceTypeFlagsOffset = Pickup->GetOffset("PickupSourceTypeFlags", false);

	if (PickupSourceTypeFlagsOffset != -1)
	{
		// Pickup->Get<uint32>(PickupSourceTypeFlagsOffset) = (uint32)PickupData.SourceType; // Assuming its the same enum on older versions.
	}

	if (Pickup->Get<AFortPawn*>(PawnWhoDroppedPickupOffset))
	{
		// TODO Add EFortItemEntryState::DroppedFromPickup or whatever if it isn't found already.
	}

	PrimaryPickupItemEntry->GetCount() = PickupData.OverrideCount == -1 ? PickupData.ItemEntry->GetCount() : PickupData.OverrideCount;

	auto PickupLocationData = Pickup->GetPickupLocationData();

	auto CanCombineWithPickup = [&](AActor* OtherPickupActor) -> bool
	{
		auto OtherPickup = (AFortPickup*)OtherPickupActor;

		if (OtherPickup == PickupData.IgnoreCombineTarget || Pickup->GetPickupLocationData()->GetCombineTarget())
			return false;

		if (OtherPickup->GetPickupLocationData()->GetTossState() != EFortPickupTossState::AtRest)
			return false;

		if (PrimaryPickupItemEntry->GetItemDefinition() == OtherPickup->GetPrimaryPickupItemEntry()->GetItemDefinition())
		{
			if (OtherPickup->GetPrimaryPickupItemEntry()->GetCount() >= PrimaryPickupItemEntry->GetItemDefinition()->GetMaxStackSize()) // Other pickup is already at the max size.
				return false;

			return true;
		}

		return false;
	};

	if (Addresses::CombinePickupLea && bEnableCombinePickup)
	{
		PickupLocationData->GetCombineTarget() = (AFortPickup*)Pickup->GetClosestActor(AFortPickup::StaticClass(), 4, CanCombineWithPickup);
	}

	if (!PickupLocationData->GetCombineTarget()) // I don't think we should call TossPickup for every pickup anyways.
	{
		Pickup->TossPickup(PickupData.SpawnLocation, PickupData.PawnOwner, 0, PickupData.bToss, PickupData.SourceType, PickupData.Source);
	}
	else
	{
		auto ActorLocation = Pickup->GetActorLocation();
		auto CurrentActorLocation = PickupLocationData->GetCombineTarget()->GetActorLocation();

		int Dist = float(sqrtf(powf(CurrentActorLocation.X - ActorLocation.X, 2.0) + powf(CurrentActorLocation.Y - ActorLocation.Y, 2.0) + powf(CurrentActorLocation.Z - ActorLocation.Z, 2.0))) / 100.f;

		// LOG_INFO(LogDev, "Distance: {}", Dist);

		// our little remake of tosspickup

		PickupLocationData->GetLootFinalPosition() = PickupData.SpawnLocation;
		PickupLocationData->GetLootInitialPosition() = Pickup->GetActorLocation();
		PickupLocationData->GetFlyTime() = 1.f / Dist; // Higher the dist quicker it should be. // not right
		PickupLocationData->GetItemOwner() = PickupData.PawnOwner;
		PickupLocationData->GetFinalTossRestLocation() = PickupLocationData->GetCombineTarget()->GetActorLocation();

		Pickup->OnRep_PickupLocationData();
		Pickup->ForceNetUpdate();
	}

	if (EFortPickupSourceTypeFlag::GetEnum() && ((PickupData.SourceType & EFortPickupSourceTypeFlag::GetContainerValue()) != 0)) // crashes if we do this then tosspickup
	{
		static auto bTossedFromContainerOffset = Pickup->GetOffset("bTossedFromContainer");
		Pickup->Get<bool>(bTossedFromContainerOffset) = true;
		// Pickup->OnRep_TossedFromContainer();
	}

	if (Fortnite_Version < 6)
	{
		Pickup->SpawnMovementComponent();
	}

	return Pickup;
}

AFortPickup* AFortPickup::SpawnPickup(FFortItemEntry* ItemEntry, FVector Location,
	uint8 PickupSource, uint8 SpawnSource,
	class AFortPawn* Pawn, UClass* OverrideClass, bool bToss, int OverrideCount, AFortPickup* IgnoreCombinePickup)
{
	PickupCreateData CreateData;
	CreateData.ItemEntry = ItemEntry;
	CreateData.SpawnLocation = Location;
	CreateData.Source = SpawnSource;
	CreateData.SourceType = PickupSource;
	CreateData.PawnOwner = Pawn;
	CreateData.OverrideClass = OverrideClass;
	CreateData.bToss = bToss;
	CreateData.IgnoreCombineTarget = IgnoreCombinePickup;
	CreateData.OverrideCount = OverrideCount;

	return AFortPickup::SpawnPickup(CreateData);
}

void AFortPickup::CombinePickupHook(AFortPickup* Pickup)
{
	// LOG_INFO(LogDev, "CombinePickupHook!");

	auto PickupToCombineInto = (AFortPickup*)Pickup->GetPickupLocationData()->GetCombineTarget();

	if (PickupToCombineInto->IsActorBeingDestroyed())
		return;

	const int IncomingCount = Pickup->GetPrimaryPickupItemEntry()->GetCount();
	const int OriginalCount = PickupToCombineInto->GetPrimaryPickupItemEntry()->GetCount();

	// add more checks?

	auto ItemDefinition = PickupToCombineInto->GetPrimaryPickupItemEntry()->GetItemDefinition();

	int CountToAdd = IncomingCount;

	if (OriginalCount + CountToAdd > ItemDefinition->GetMaxStackSize())
	{
		const int OverStackCount = OriginalCount + CountToAdd - ItemDefinition->GetMaxStackSize();
		CountToAdd = ItemDefinition->GetMaxStackSize() - OriginalCount;

		auto ItemOwner = Pickup->GetPickupLocationData()->GetItemOwner();

		PickupCreateData CreateData;
		CreateData.ItemEntry = FFortItemEntry::MakeItemEntry(ItemDefinition, OverStackCount, 0, MAX_DURABILITY, Pickup->GetPrimaryPickupItemEntry()->GetLevel());
		CreateData.SpawnLocation = PickupToCombineInto->GetActorLocation();
		CreateData.PawnOwner = ItemOwner;
		CreateData.SourceType = EFortPickupSourceTypeFlag::GetPlayerValue();
		CreateData.IgnoreCombineTarget = PickupToCombineInto;
		CreateData.bShouldFreeItemEntryWhenDeconstructed = true;

		auto NewOverStackPickup = AFortPickup::SpawnPickup(CreateData);
	}

	PickupToCombineInto->GetPrimaryPickupItemEntry()->GetCount() += CountToAdd;
	PickupToCombineInto->OnRep_PrimaryPickupItemEntry();

	PickupToCombineInto->ForceNetUpdate();
	PickupToCombineInto->FlushNetDormancy();

	Pickup->K2_DestroyActor();
}

char AFortPickup::CompletePickupAnimationHook(AFortPickup* Pickup)
{
	auto Pawn = Cast<AFortPlayerPawn>(Pickup->GetPickupLocationData()->GetPickupTarget());

	if (!Pawn)
		return CompletePickupAnimationOriginal(Pickup);

	auto PlayerController = Cast<AFortPlayerController>(Pawn->GetController());

	if (!PlayerController)
		return CompletePickupAnimationOriginal(Pickup);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return CompletePickupAnimationOriginal(Pickup);

	auto PickupEntry = Pickup->GetPrimaryPickupItemEntry();

	auto PickupItemDefinition = Cast<UFortWorldItemDefinition>(PickupEntry->GetItemDefinition());
	auto IncomingCount = PickupEntry->GetCount();

	if (!PickupItemDefinition)
		return CompletePickupAnimationOriginal(Pickup);

	if (auto GameModePickup = Cast<AFortGameModePickup>(Pickup))
	{
		LOG_INFO(LogDev, "GameModePickup!");
		return CompletePickupAnimationOriginal(Pickup);
	}

	auto& ItemInstances = WorldInventory->GetItemList().GetItemInstances();

	auto& CurrentItemGuid = Pickup->GetPickupLocationData()->GetPickupGuid(); // Pawn->CurrentWeapon->ItemEntryGuid;

	auto ItemInstanceToSwap = WorldInventory->FindItemInstance(CurrentItemGuid);

	// if (!ItemInstanceToSwap)
		// return CompletePickupAnimationOriginal(Pickup);

	auto ItemEntryToSwap = ItemInstanceToSwap ? ItemInstanceToSwap->GetItemEntry() : nullptr;
	auto ItemDefinitionToSwap = ItemEntryToSwap ? Cast<UFortWorldItemDefinition>(ItemEntryToSwap->GetItemDefinition()) : nullptr;
	bool bHasSwapped = false;

	int cpyCount = IncomingCount;

	auto PawnLoc = Pawn->GetActorLocation();
	auto ItemDefGoingInPrimary = IsPrimaryQuickbar(PickupItemDefinition);

	std::vector<std::pair<FFortItemEntry*, FFortItemEntry*>> PairsToMarkDirty; // vector of sets or something so no duplicates??
	
	if (bDebugPrintSwapping)
		LOG_INFO(LogDev, "Start cpyCount: {}", cpyCount);

	bool bWasHoldingSameItemWhenSwap = false;

	FGuid NewSwappedItem = FGuid(-1, -1, -1, -1);

	bool bForceDontAddItem = false;
	bool bForceOverflow = false;

	while (cpyCount > 0)
	{
		int PrimarySlotsFilled = 0;
		bool bEverStacked = false;
		bool bDoesStackExist = false;

		bool bIsInventoryFull = false;

		for (int i = 0; i < ItemInstances.Num(); ++i)
		{
			auto ItemInstance = ItemInstances.at(i);
			auto CurrentItemEntry = ItemInstance->GetItemEntry();

			if (ItemDefGoingInPrimary && IsPrimaryQuickbar(CurrentItemEntry->GetItemDefinition()))
			{
				int AmountOfSlotsTakenUp = 1; // TODO
				PrimarySlotsFilled += AmountOfSlotsTakenUp;
			}

			// LOG_INFO(LogDev, "[{}] PrimarySlotsFilled: {}", i, PrimarySlotsFilled);

			bIsInventoryFull = (PrimarySlotsFilled /* - 6 */) >= 5;

			if (bIsInventoryFull || (PlayerController->HasTryPickupSwap() ? PlayerController->ShouldTryPickupSwap() : false)) // probs shouldnt do in loop but alr
			{
				if (PlayerController->HasTryPickupSwap())
					PlayerController->ShouldTryPickupSwap() = false;

				if (ItemInstanceToSwap && ItemDefinitionToSwap->CanBeDropped() && !bHasSwapped && ItemDefGoingInPrimary) // swap
				{
					auto SwappedPickup = SpawnPickup(ItemEntryToSwap, PawnLoc, EFortPickupSourceTypeFlag::GetPlayerValue(), 0, Pawn);

					auto CurrentWeapon = Pawn->GetCurrentWeapon();

					if (CurrentWeapon)
					{
						bWasHoldingSameItemWhenSwap = CurrentWeapon->GetItemEntryGuid()  == ItemInstanceToSwap->GetItemEntry()->GetItemGuid();
					}

					// THIS IS NOT PROPER! We should use the commented code but there are some bugs idk why.

					WorldInventory->RemoveItem(CurrentItemGuid, nullptr, ItemEntryToSwap->GetCount(), true);

					/*
					auto NewItemCount = cpyCount > PickupItemDefinition->GetMaxStackSize() ? PickupItemDefinition->GetMaxStackSize() : cpyCount;

					std::pair<FFortItemEntry*, FFortItemEntry*> Pairs;
					WorldInventory->SwapItem(CurrentItemGuid, PickupEntry, cpyCount, &Pairs);
					PairsToMarkDirty.push_back(Pairs);

					cpyCount -= NewItemCount;
					*/

					bHasSwapped = true;

					if (bDebugPrintSwapping)
						LOG_INFO(LogDev, "[{}] Swapping: {}", i, ItemDefinitionToSwap->GetFullName());

					// bForceDontAddItem = true;

					continue; // ???
				}
			}

			if (CurrentItemEntry->GetItemDefinition() == PickupItemDefinition)
			{
				if (bDebugPrintSwapping)
					LOG_INFO(LogDev, "[{}] Found stack of item!", i);

				if (CurrentItemEntry->GetCount() < PickupItemDefinition->GetMaxStackSize())
				{
					int OverStack = CurrentItemEntry->GetCount() + cpyCount - PickupItemDefinition->GetMaxStackSize();
					int AmountToStack = OverStack > 0 ? cpyCount - OverStack : cpyCount;

					cpyCount -= AmountToStack;

					std::pair<FFortItemEntry*, FFortItemEntry*> Pairs;
					WorldInventory->ModifyCount(ItemInstance, AmountToStack, false, &Pairs, false, true);
					PairsToMarkDirty.push_back(Pairs);

					bEverStacked = true;

					if (bDebugPrintSwapping)
						LOG_INFO(LogDev, "[{}] We are stacking {}.", i, AmountToStack);

					// if (cpyCount > 0)
						// break;
				}

				bDoesStackExist = true;
			}

			if ((bIsInventoryFull || bForceOverflow) && cpyCount > 0) // overflow
			{
				if (bDebugPrintSwapping)
					LOG_INFO(LogDev, "[{}] Overflow", i);

				UFortWorldItemDefinition* ItemDefinitionToSpawn = PickupItemDefinition;
				int AmountToSpawn = cpyCount > PickupItemDefinition->GetMaxStackSize() ? PickupItemDefinition->GetMaxStackSize() : cpyCount;

				int LoadedAmmo = 0;

				// SpawnPickup(ItemDefinitionToSpawn, PawnLoc, AmountToSpawn, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, -1, Pawn);
				SpawnPickup(PickupEntry, PawnLoc, EFortPickupSourceTypeFlag::GetPlayerValue(), 0, Pawn, nullptr, true, AmountToSpawn);
				cpyCount -= AmountToSpawn;
				bForceOverflow = false;
			}

			if (cpyCount <= 0)
				break;
		}

		if (cpyCount > 0 && !bIsInventoryFull && !bForceDontAddItem)
		{
			if (bDebugPrintSwapping)
				LOG_INFO(LogDev, "Attempting to add to inventory.");

			if (bDoesStackExist ? PickupItemDefinition->DoesAllowMultipleStacks() : true)
			{
				auto NewItemCount = cpyCount > PickupItemDefinition->GetMaxStackSize() ? PickupItemDefinition->GetMaxStackSize() : cpyCount;

				auto NewAndModifiedInstances = WorldInventory->AddItem(PickupEntry, nullptr, true, NewItemCount);

				auto NewVehicleInstance = NewAndModifiedInstances.first[0];

				if (!NewVehicleInstance)
					continue;
				else
					cpyCount -= NewItemCount;

				if (bDebugPrintSwapping)
					LOG_INFO(LogDev, "Added item with count {} to inventory.", NewItemCount);

				if (bHasSwapped && NewSwappedItem == FGuid(-1, -1, -1, -1))
				{
					NewSwappedItem = NewVehicleInstance->GetItemEntry()->GetItemGuid();
				}
			}
			else
			{
				bForceOverflow = true;
			}
		}
	}

	// auto Item = GiveItem(PlayerController, ItemDef, cpyCount, CurrentPickup->PrimaryPickupItemEntry.LoadedAmmo, true);

	/* for (int i = 0; i < Pawn->IncomingPickups.Num(); ++i)
	{
		Pawn->IncomingPickups[i]->PickupLocationData.PickupGuid = Item->ItemEntry.ItemGuid;
	} */

	WorldInventory->Update(PairsToMarkDirty.size() == 0);

	for (auto& [key, value] : PairsToMarkDirty)
	{
		WorldInventory->GetItemList().MarkItemDirty(key);
		WorldInventory->GetItemList().MarkItemDirty(value);
	}

	if (bWasHoldingSameItemWhenSwap && NewSwappedItem != FGuid(-1, -1, -1, -1))
	{
		PlayerController->ClientEquipItem(NewSwappedItem, true);
	}

	return CompletePickupAnimationOriginal(Pickup);
}

UClass* AFortPickup::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/FortniteGame.FortPickup");
	return Class;
}
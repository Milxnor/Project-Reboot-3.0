#include "FortPlayerPawnAthena.h"
#include "FortInventory.h"
#include "FortPlayerControllerAthena.h"

void AFortPlayerPawnAthena::OnCapsuleBeginOverlapHook(UObject* Context, FFrame* Stack, void* Ret)
{
	using UPrimitiveComponent = UObject;

	auto Pawn = (AFortPlayerPawnAthena*)Context;
	UPrimitiveComponent* OverlappedComp;
	AActor* OtherActor;
	UPrimitiveComponent* OtherComp;
	int OtherBodyIndex;
	bool bFromSweep;
	FHitResult SweepResult = *Alloc<FHitResult>(FHitResult::GetStructSize());

	LOG_INFO(LogDev, "OnCapsuleBeginOverlapHook!");

	Stack->StepCompiledIn(&OverlappedComp);
	Stack->StepCompiledIn(&OtherActor);
	Stack->StepCompiledIn(&OtherComp);
	Stack->StepCompiledIn(&OtherBodyIndex);
	Stack->StepCompiledIn(&bFromSweep);
	Stack->StepCompiledIn(&SweepResult);

	if (auto Pickup = Cast<AFortPickup>(OtherActor))
	{
		static auto PawnWhoDroppedPickupOffset = Pickup->GetOffset("PawnWhoDroppedPickup");

		if (Pickup->Get<AFortPawn*>(PawnWhoDroppedPickupOffset) != Pawn)
		{
			auto ItemDefinition = Pickup->GetPrimaryPickupItemEntry()->GetItemDefinition();

			auto  PlayerController = Cast<AFortPlayerControllerAthena>(Pawn->GetController(), false);
			auto& ItemInstances = PlayerController->GetWorldInventory()->GetItemList().GetItemInstances();

			bool  ItemDefGoingInPrimary = IsPrimaryQuickbar(ItemDefinition);
			int   PrimarySlotsFilled = 0;
			bool  bCanStack = false;
			bool  bFoundStack = false;

			for (int i = 0; i < ItemInstances.Num(); i++)
			{
				auto ItemInstance = ItemInstances.at(i);
				auto CurrentItemDefinition = ItemInstance->GetItemEntry()->GetItemDefinition();

				if (ItemDefGoingInPrimary && IsPrimaryQuickbar(CurrentItemDefinition))
					PrimarySlotsFilled++;

				bool bIsInventoryFull = (PrimarySlotsFilled /* - 6 */) >= 5;

				if (CurrentItemDefinition == ItemDefinition)
				{
					bFoundStack = true;

					if (ItemInstance->GetItemEntry()->GetCount() < ItemDefinition->GetMaxStackSize())
					{
						bCanStack = true;
						break;
					}
				}

				if (bIsInventoryFull)
					return OnCapsuleBeginOverlapOriginal(Context, Stack, Ret);
			}

			// std::cout << "bCanStack: " << bCanStack << '\n';
			// std::cout << "bFoundStack: " << bFoundStack << '\n';

			if (!bCanStack ? (!bFoundStack ? true : ItemDefinition->DoesAllowMultipleStacks()) : true)
				ServerHandlePickupHook(Pawn, Pickup, 0.4, FVector(), true);
		}
	}

	return OnCapsuleBeginOverlapOriginal(Context, Stack, Ret);
}
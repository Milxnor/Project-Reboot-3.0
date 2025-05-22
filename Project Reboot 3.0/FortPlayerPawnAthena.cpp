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
	auto SweepResultPtr = (FHitResult*)std::realloc(0, FHitResult::GetStructSize());

	LOG_INFO(LogDev, "OnCapsuleBeginOverlapHook!");

	Stack->StepCompiledIn(&OverlappedComp);
	Stack->StepCompiledIn(&OtherActor);
	Stack->StepCompiledIn(&OtherComp);
	Stack->StepCompiledIn(&OtherBodyIndex);
	Stack->StepCompiledIn(&bFromSweep);
	Stack->StepCompiledIn(SweepResultPtr);

	std::free(SweepResultPtr);

	// LOG_INFO(LogDev, "OtherActor: {}", __int64(OtherActor));
	// LOG_INFO(LogDev, "OtherActorName: {}", OtherActor->IsValidLowLevel() ? OtherActor->GetName() : "BadRead")
	
	if (!Pawn->IsDBNO())
	{
		if (auto Pickup = Cast<AFortPickup>(OtherActor))
		{
			static auto PawnWhoDroppedPickupOffset = Pickup->GetOffset("PawnWhoDroppedPickup");

			if (Pickup->Get<AFortPawn*>(PawnWhoDroppedPickupOffset) != Pawn)
			{
				auto ItemDefinition = Pickup->GetPrimaryPickupItemEntry()->GetItemDefinition();

				if (!ItemDefinition)
				{
					return;
				}

				if (!IsPrimaryQuickbar(ItemDefinition))
				{
					ServerHandlePickupHook(Pawn, Pickup, 0.4f, FVector(), true);
				}
			}
		}
	}

	if (OtherActor && OtherActor->GetName().contains("Rift") && Fortnite_Version >= 17)
	{
		Pawn->TeleportTo(Pawn->GetActorLocation() + FVector(0, 0, 20000), Pawn->GetActorRotation());

		UAbilitySystemComponent* AbilitySystemComponent = ((AFortPlayerStateAthena*)Pawn->GetPlayerState())->GetAbilitySystemComponent();
		if (!AbilitySystemComponent) return;

		static UClass* RiftAbility = FindObject<UClass>(L"/Game/Athena/Items/ForagedItems/Rift/GA_Rift_Athena_Skydive.GA_Rift_Athena_Skydive_C");
		if (!RiftAbility) return;

		FGameplayAbilitySpec* Spec = MakeNewSpec(RiftAbility);
		if (!Spec) return;

		static unsigned int* (*GiveAbilityAndActivateOnce)(UAbilitySystemComponent * ASC, int* outHandle, __int64 Spec, FGameplayEventData * TriggerEventData) = decltype(GiveAbilityAndActivateOnce)(Addresses::GiveAbilityAndActivateOnce); 

		if (GiveAbilityAndActivateOnce)
		{
			int outHandle = 0;
			GiveAbilityAndActivateOnce(AbilitySystemComponent, &outHandle, __int64(Spec), nullptr);
		}
	}

	// return OnCapsuleBeginOverlapOriginal(Context, Stack, Ret); // we love explicit
}
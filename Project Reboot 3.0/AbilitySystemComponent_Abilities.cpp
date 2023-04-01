#include "AbilitySystemComponent.h"
#include "NetSerialization.h"
#include "Actor.h"
#include "FortPawn.h"
#include "FortPlayerController.h"
#include "FortPlayerStateAthena.h"

void LoopSpecs(UAbilitySystemComponent* AbilitySystemComponent, std::function<void(FGameplayAbilitySpec*)> func)
{
	static auto ActivatableAbilitiesOffset = AbilitySystemComponent->GetOffset("ActivatableAbilities");
	auto ActivatableAbilities = AbilitySystemComponent->GetPtr<FFastArraySerializer>(ActivatableAbilitiesOffset);

	static auto ItemsOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpecContainer", "Items");
	auto Items = (TArray<FGameplayAbilitySpec>*)(__int64(ActivatableAbilities) + ItemsOffset);

	static auto SpecSize = FGameplayAbilitySpec::GetStructSize();

	if (ActivatableAbilities && Items)
	{
		for (int i = 0; i < Items->Num(); i++)
		{
			auto CurrentSpec = Items->AtPtr(i, SpecSize); // (FGameplayAbilitySpec*)(__int64(Items->Data) + (static_cast<long long>(SpecSize) * i));
			func(CurrentSpec);
		}
	}
}

void UAbilitySystemComponent::RemoveActiveGameplayEffectBySourceEffect(UClass* GEClass, int StacksToRemove, UAbilitySystemComponent* Instigator)
{
	static auto RemoveActiveGameplayEffectBySourceEffectFn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.RemoveActiveGameplayEffectBySourceEffect");
	struct { UClass* GameplayEffect; UObject* InstigatorAbilitySystemComponent; int StacksToRemove; } UAbilitySystemComponent_RemoveActiveGameplayEffectBySourceEffect_Params{ GEClass, Instigator, StacksToRemove };

	this->ProcessEvent(RemoveActiveGameplayEffectBySourceEffectFn, &UAbilitySystemComponent_RemoveActiveGameplayEffectBySourceEffect_Params);
}

void UAbilitySystemComponent::ConsumeAllReplicatedData(FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey* AbilityOriginalPredictionKey)
{
	// static auto AbilityTargetDataMapOffset = ActivatableAbilitiesOffset + FGameplayAbilitySpecContainerSize ?

	return;
}

void UAbilitySystemComponent::InternalServerTryActivateAbilityHook(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey* PredictionKey, const FGameplayEventData* TriggerEventData) // https://github.com/EpicGames/UnrealEngine/blob/4.23/Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/AbilitySystemComponent_Abilities.cpp#L1445
{
	using UGameplayAbility = UObject;

	auto Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);

	static auto PredictionKeyStruct = FindObject<UStruct>("/Script/GameplayAbilities.PredictionKey");
	static auto PredictionKeySize = PredictionKeyStruct->GetPropertiesSize();
	static auto CurrentOffset = FindOffsetStruct("/Script/GameplayAbilities.PredictionKey", "Current");

	if (!Spec)
	{
		LOG_INFO(LogAbilities, "InternalServerTryActivateAbility. Rejecting ClientActivation of ability with invalid SpecHandle!");
		AbilitySystemComponent->ClientActivateAbilityFailed(Handle, *(int16_t*)(__int64(PredictionKey) + CurrentOffset));
		return;
	}

	AbilitySystemComponent->ConsumeAllReplicatedData(Handle, (FPredictionKey*)PredictionKey);

	/* const */ UGameplayAbility * AbilityToActivate = Spec->GetAbility();

	if (!AbilityToActivate)
	{
		LOG_ERROR(LogAbilities, "InternalServerTryActiveAbility. Rejecting ClientActivation of unconfigured spec ability!");
		AbilitySystemComponent->ClientActivateAbilityFailed(Handle, *(int16_t*)(__int64(PredictionKey) + CurrentOffset));
		return;
	}

	static auto InputPressedOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpec", "InputPressed");

	UGameplayAbility* InstancedAbility = nullptr;
	SetBitfield((PlaceholderBitfield*)(__int64(Spec) + InputPressedOffset), 1, true); // InputPressed = true
	
	bool res = false;

	if (PredictionKeySize == 0x10)
		res = UAbilitySystemComponent::InternalTryActivateAbilityOriginal(AbilitySystemComponent, Handle, *(PadHex10*)PredictionKey, &InstancedAbility, nullptr, TriggerEventData);
	else if (PredictionKeySize == 0x18)
		res = UAbilitySystemComponent::InternalTryActivateAbilityOriginal2(AbilitySystemComponent, Handle, *(PadHex18*)PredictionKey, &InstancedAbility, nullptr, TriggerEventData);
	else
		LOG_ERROR(LogAbilities, "Prediction key size does not match with any of them!");

	if (!res)
	{
		LOG_INFO(LogAbilities, "InternalServerTryActivateAbility. Rejecting ClientActivation of {}. InternalTryActivateAbility failed: ", AbilityToActivate->GetName());

		AbilitySystemComponent->ClientActivateAbilityFailed(Handle, *(int16_t*)(__int64(PredictionKey) + CurrentOffset));
		SetBitfield((PlaceholderBitfield*)(__int64(Spec) + InputPressedOffset), 1, false); // InputPressed = false

		static auto ActivatableAbilitiesOffset = AbilitySystemComponent->GetOffset("ActivatableAbilities");
		AbilitySystemComponent->Get<FFastArraySerializer>(ActivatableAbilitiesOffset).MarkItemDirty(Spec); 
	}
	else
	{
		// LOG_INFO(LogAbilities, "InternalServerTryActivateAbility. Activated {}", AbilityToActivate->GetName());
	}

	// bro ignore this next part idk where to put it ok

	/* static auto OwnerActorOffset = AbilitySystemComponent->GetOffset("OwnerActor");
	auto PlayerState = Cast<AFortPlayerStateAthena>(AbilitySystemComponent->Get<AActor*>(OwnerActorOffset));

	if (!PlayerState)
		return;

	auto Controller = Cast<AFortPlayerController>(PlayerState->GetOwner());
	LOG_INFO(LogAbilities, "Owner {}", PlayerState->GetOwner()->GetFullName());

	if (!Controller)
		return;

	auto Pawn = Controller->GetMyFortPawn();

	if (!Pawn)
		return;

	auto CurrentWeapon = Pawn->GetCurrentWeapon();
	auto WorldInventory = Controller ? Controller->GetWorldInventory() : nullptr;

	if (!WorldInventory || !CurrentWeapon)
		return;

	auto CurrentWeaponInstance = WorldInventory->FindItemInstance(CurrentWeapon->GetItemEntryGuid());
	auto CurrentWeaponReplicatedEntry = WorldInventory->FindReplicatedEntry(CurrentWeapon->GetItemEntryGuid());

	static auto AmmoCountOffset = CurrentWeapon->GetOffset("AmmoCount");
	auto AmmoCount = CurrentWeapon->Get<int>(AmmoCountOffset);

	if (CurrentWeaponReplicatedEntry->GetLoadedAmmo() != AmmoCount)
	{
		CurrentWeaponInstance->GetItemEntry()->GetLoadedAmmo() = AmmoCount;
		CurrentWeaponReplicatedEntry->GetLoadedAmmo() = AmmoCount;

		WorldInventory->GetItemList().MarkItemDirty(CurrentWeaponInstance->GetItemEntry());
		WorldInventory->GetItemList().MarkItemDirty(CurrentWeaponReplicatedEntry);
	} */
}

FGameplayAbilitySpecHandle UAbilitySystemComponent::GiveAbilityEasy(UClass* AbilityClass)
{
	// LOG_INFO(LogDev, "Making spec!");

	auto NewSpec = MakeNewSpec(AbilityClass);

	// LOG_INFO(LogDev, "Made spec!");

	FGameplayAbilitySpecHandle Handle;

	GiveAbilityOriginal(this, &Handle, __int64(NewSpec));

	return Handle;
}

FGameplayAbilitySpec* UAbilitySystemComponent::FindAbilitySpecFromHandle(FGameplayAbilitySpecHandle Handle)
{
	FGameplayAbilitySpec* SpecToReturn = nullptr;

	auto compareHandles = [&Handle, &SpecToReturn](FGameplayAbilitySpec* Spec) {
		auto& CurrentHandle = Spec->GetHandle();

		if (CurrentHandle.Handle == Handle.Handle)
		{
			SpecToReturn = Spec;
			return;
		}
	};

	LoopSpecs(this, compareHandles);

	return SpecToReturn;
}
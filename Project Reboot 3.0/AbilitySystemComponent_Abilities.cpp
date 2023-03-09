#include "AbilitySystemComponent.h"
#include "NetSerialization.h"

void LoopSpecs(UAbilitySystemComponent* AbilitySystemComponent, std::function<void(FGameplayAbilitySpec*)> func)
{
	static auto ActivatableAbilitiesOffset = AbilitySystemComponent->GetOffset("ActivatableAbilities");
	auto ActivatableAbilities = AbilitySystemComponent->GetPtr<FFastArraySerializer>(ActivatableAbilitiesOffset);

	static auto ItemsOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpecContainer", "Items");
	auto Items = (TArray<__int64>*)(__int64(ActivatableAbilities) + ItemsOffset);

	static auto SpecSize = FGameplayAbilitySpec::GetStructSize();

	if (ActivatableAbilities && Items)
	{
		for (int i = 0; i < Items->Num(); i++)
		{
			auto CurrentSpec = (FGameplayAbilitySpec*)(__int64(Items->Data) + (static_cast<long long>(SpecSize) * i));
			func(CurrentSpec);
		}
	}
}

void InternalServerTryActivateAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, /* bool InputPressed, */
	const FPredictionKey* PredictionKey, const FGameplayEventData* TriggerEventData) // https://github.com/EpicGames/UnrealEngine/blob/4.23/Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/AbilitySystemComponent_Abilities.cpp#L1445
{
	using UGameplayAbility = UObject;

	auto Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);

	static auto CurrentOffset = FindOffsetStruct("/Script/GameplayAbilities.PredictionKey", "Current");

	if (!Spec)
	{
		AbilitySystemComponent->ClientActivateAbilityFailed(Handle, *(int16_t*)(__int64(PredictionKey) + CurrentOffset));
		return;
	}

	// ConsumeAllReplicatedData(Handle, PredictionKey);

	const UGameplayAbility* AbilityToActivate = Spec->GetAbility();

	if (!AbilityToActivate)
		return;

	UGameplayAbility* InstancedAbility = nullptr;
	SetBitfield(Spec, 1, true); // InputPressed = true

	if (!AbilitySystemComponent->InternalTryActivateAbilityOriginal(AbilitySystemComponent, Handle, *(FPredictionKey*)PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
	{
		AbilitySystemComponent->ClientActivateAbilityFailed(Handle, *(int16_t*)(__int64(PredictionKey) + CurrentOffset));
		SetBitfield(Spec, 1, false); // InputPressed = false

		static auto ActivatableAbilitiesOffset = AbilitySystemComponent->GetOffset("ActivatableAbilities");
		AbilitySystemComponent->Get<FFastArraySerializer>(ActivatableAbilitiesOffset).MarkItemDirty(Spec);
	}
}

FGameplayAbilitySpecHandle UAbilitySystemComponent::GiveAbilityEasy(UClass* AbilityClass)
{
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

void UAbilitySystemComponent::ServerTryActivateAbilityHook(UAbilitySystemComponent* AbilitySystemComponent, 
	FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey PredictionKey)
{
	InternalServerTryActivateAbility(AbilitySystemComponent, Handle, /* InputPressed, */ &PredictionKey, nullptr);
}

void UAbilitySystemComponent::ServerTryActivateAbilityWithEventDataHook(UAbilitySystemComponent* AbilitySystemComponent,
	FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey PredictionKey, FGameplayEventData TriggerEventData)
{
	InternalServerTryActivateAbility(AbilitySystemComponent, Handle, /* InputPressed, */
		&PredictionKey, &TriggerEventData);
}

void UAbilitySystemComponent::ServerAbilityRPCBatchHook(UAbilitySystemComponent* AbilitySystemComponent, __int64 BatchInfo)
{
	static auto AbilitySpecHandleOffset = 0x0;
	static auto PredictionKeyOffset = 0x0008;

	InternalServerTryActivateAbility(AbilitySystemComponent, *(FGameplayAbilitySpecHandle*)(__int64(BatchInfo) + AbilitySpecHandleOffset),
		/* InputPressed, */ (FPredictionKey*)(__int64(BatchInfo) + PredictionKeyOffset), nullptr);
}
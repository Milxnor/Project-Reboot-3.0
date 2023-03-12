#include "AbilitySystemComponent.h"
#include "NetSerialization.h"

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

void InternalServerTryActivateAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, /* bool InputPressed, */
	const FPredictionKey* PredictionKey, const FGameplayEventData* TriggerEventData) // https://github.com/EpicGames/UnrealEngine/blob/4.23/Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Private/AbilitySystemComponent_Abilities.cpp#L1445
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

	// ConsumeAllReplicatedData(Handle, PredictionKey);

	/* const */ UGameplayAbility * AbilityToActivate = Spec->GetAbility();

	if (!AbilityToActivate)
	{
		LOG_ERROR(LogAbilities, "InternalServerTryActiveAbility. Rejecting ClientActivation of unconfigured spec ability!");
		AbilitySystemComponent->ClientActivateAbilityFailed(Handle, *(int16_t*)(__int64(PredictionKey) + CurrentOffset));
		return;
	}

	UGameplayAbility* InstancedAbility = nullptr;
	SetBitfield(Spec, 1, true); // InputPressed = true
	
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
		SetBitfield(Spec, 1, false); // InputPressed = false
	}
	else
	{
		LOG_INFO(LogAbilities, "InternalServerTryActivateAbility. Activated {}", AbilityToActivate->GetName());
	}

	static auto ActivatableAbilitiesOffset = AbilitySystemComponent->GetOffset("ActivatableAbilities");
	AbilitySystemComponent->Get<FFastArraySerializer>(ActivatableAbilitiesOffset).MarkItemDirty(Spec); // we only need to do this if the ability fails but eh
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

void UAbilitySystemComponent::ServerTryActivateAbilityHook1(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, PadHex10 PredictionKey)
{
	LOG_INFO(LogAbilities, "ServerTryActivateAbility1");
	InternalServerTryActivateAbility(AbilitySystemComponent, Handle, /* InputPressed, */ (FPredictionKey*)&PredictionKey, nullptr);
}

void UAbilitySystemComponent::ServerTryActivateAbilityHook2(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, PadHex18 PredictionKey)
{
	LOG_INFO(LogAbilities, "ServerTryActivateAbility2");
	InternalServerTryActivateAbility(AbilitySystemComponent, Handle, /* InputPressed, */ (FPredictionKey*)&PredictionKey, nullptr);
}

void UAbilitySystemComponent::ServerTryActivateAbilityWithEventDataHook1(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, PadHex10 PredictionKey, FGameplayEventData TriggerEventData)
{
	LOG_INFO(LogAbilities, "ServerTryActivateAbilityWithEventData1");
	InternalServerTryActivateAbility(AbilitySystemComponent, Handle, /* InputPressed, */
		(FPredictionKey*)&PredictionKey, &TriggerEventData);
}

void UAbilitySystemComponent::ServerTryActivateAbilityWithEventDataHook2(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, PadHex18 PredictionKey, FGameplayEventData TriggerEventData)
{
	LOG_INFO(LogAbilities, "ServerTryActivateAbilityWithEventData2");
	InternalServerTryActivateAbility(AbilitySystemComponent, Handle, /* InputPressed, */
		(FPredictionKey*)&PredictionKey, &TriggerEventData);
}

void UAbilitySystemComponent::ServerAbilityRPCBatchHook(UAbilitySystemComponent* AbilitySystemComponent, __int64 BatchInfo)
{
	static auto AbilitySpecHandleOffset = 0x0;
	static auto PredictionKeyOffset = 0x0008;

	InternalServerTryActivateAbility(AbilitySystemComponent, *(FGameplayAbilitySpecHandle*)(__int64(BatchInfo) + AbilitySpecHandleOffset),
		/* InputPressed, */ (FPredictionKey*)(__int64(BatchInfo) + PredictionKeyOffset), nullptr);
}
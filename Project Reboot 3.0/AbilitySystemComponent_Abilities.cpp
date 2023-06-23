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
		for (int i = 0; i < Items->Num(); ++i)
		{
			auto CurrentSpec = Items->AtPtr(i, SpecSize); // (FGameplayAbilitySpec*)(__int64(Items->Data) + (static_cast<long long>(SpecSize) * i));
			func(CurrentSpec);
		}
	}
}

FActiveGameplayEffectHandle UAbilitySystemComponent::ApplyGameplayEffectToSelf(UClass* GameplayEffectClass, float Level, const FGameplayEffectContextHandle& EffectContext)
{
	static auto BP_ApplyGameplayEffectToSelfFn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.BP_ApplyGameplayEffectToSelf");

	struct
	{
		UClass* GameplayEffectClass;                                      // (Parm, ZeroConstructor, IsPlainOldData)
		float                                              Level;                                                    // (Parm, ZeroConstructor, IsPlainOldData)
		FGameplayEffectContextHandle                EffectContext;                                            // (Parm)
		FActiveGameplayEffectHandle                 ReturnValue;                                              // (Parm, OutParm, ReturnParm)
	}UAbilitySystemComponent_BP_ApplyGameplayEffectToSelf_Params{GameplayEffectClass, Level, EffectContext};

	this->ProcessEvent(BP_ApplyGameplayEffectToSelfFn, &UAbilitySystemComponent_BP_ApplyGameplayEffectToSelf_Params);

	return UAbilitySystemComponent_BP_ApplyGameplayEffectToSelf_Params.ReturnValue;
}

/* FGameplayEffectContextHandle UAbilitySystemComponent::MakeEffectContext()
{
	static auto MakeEffectContextFn = FindObject<UFunction>("/Script/GameplayAbilities.AbilitySystemComponent.MakeEffectContext");
	FGameplayEffectContextHandle ContextHandle;
	this->ProcessEvent(MakeEffectContextFn, &ContextHandle);
	return ContextHandle;
} */

void UAbilitySystemComponent::ServerEndAbility(FGameplayAbilitySpecHandle AbilityToEnd, FGameplayAbilityActivationInfo* ActivationInfo, FPredictionKey* PredictionKey)
{
	static auto ServerEndAbilityFn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.ServerEndAbility");

	auto Params = Alloc<void>(ServerEndAbilityFn->GetPropertiesSize());

	if (!Params)
		return;

	static auto AbilityToEndOffset = FindOffsetStruct("/Script/GameplayAbilities.AbilitySystemComponent.ServerEndAbility", "AbilityToEnd");
	static auto ActivationInfoOffset = FindOffsetStruct("/Script/GameplayAbilities.AbilitySystemComponent.ServerEndAbility", "ActivationInfo");
	static auto PredictionKeyOffset = FindOffsetStruct("/Script/GameplayAbilities.AbilitySystemComponent.ServerEndAbility", "PredictionKey");

	*(FGameplayAbilitySpecHandle*)(__int64(Params) + AbilityToEndOffset) = AbilityToEnd;
	CopyStruct((FGameplayAbilityActivationInfo*)(__int64(Params) + ActivationInfoOffset), ActivationInfo, FGameplayAbilityActivationInfo::GetStructSize());
	CopyStruct((FPredictionKey*)(__int64(Params) + PredictionKeyOffset), PredictionKey, FPredictionKey::GetStructSize());

	this->ProcessEvent(ServerEndAbilityFn, Params);

	VirtualFree(Params, 0, MEM_RELEASE);
}

void UAbilitySystemComponent::ClientEndAbility(FGameplayAbilitySpecHandle AbilityToEnd, FGameplayAbilityActivationInfo* ActivationInfo)
{
	static auto ClientEndAbilityFn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.ClientEndAbility");

	auto Params = Alloc(ClientEndAbilityFn->GetPropertiesSize());

	static auto AbilityToEndOffset = FindOffsetStruct("/Script/GameplayAbilities.AbilitySystemComponent.ClientEndAbility", "AbilityToEnd");
	static auto ActivationInfoOffset = FindOffsetStruct("/Script/GameplayAbilities.AbilitySystemComponent.ClientEndAbility", "ActivationInfo");

	*(FGameplayAbilitySpecHandle*)(__int64(Params) + AbilityToEndOffset) = AbilityToEnd;
	CopyStruct((FGameplayAbilityActivationInfo*)(__int64(Params) + ActivationInfoOffset), ActivationInfo, FGameplayAbilityActivationInfo::GetStructSize());

	this->ProcessEvent(ClientEndAbilityFn, Params);

	VirtualFree(Params, 0, MEM_RELEASE);
}

void UAbilitySystemComponent::ClientCancelAbility(FGameplayAbilitySpecHandle AbilityToCancel, FGameplayAbilityActivationInfo* ActivationInfo)
{
	static auto ClientCancelAbilityFn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.ClientCancelAbility");

	auto Params = Alloc(ClientCancelAbilityFn->GetPropertiesSize());

	static auto AbilityToCancelOffset = FindOffsetStruct("/Script/GameplayAbilities.AbilitySystemComponent.ClientCancelAbility", "AbilityToCancel");
	static auto ActivationInfoOffset = FindOffsetStruct("/Script/GameplayAbilities.AbilitySystemComponent.ClientCancelAbility", "ActivationInfo");

	*(FGameplayAbilitySpecHandle*)(__int64(Params) + AbilityToCancelOffset) = AbilityToCancel;
	CopyStruct((FGameplayAbilityActivationInfo*)(__int64(Params) + ActivationInfoOffset), ActivationInfo, FGameplayAbilityActivationInfo::GetStructSize());

	this->ProcessEvent(ClientCancelAbilityFn, Params);

	VirtualFree(Params, 0, MEM_RELEASE);
}

bool UAbilitySystemComponent::HasAbility(UObject* DefaultAbility)
{
	auto ActivatableAbilities = GetActivatableAbilities();

	auto& Items = ActivatableAbilities->GetItems();

	for (int i = 0; i < Items.Num(); ++i)
	{
		auto Spec = Items.AtPtr(i, FGameplayAbilitySpec::GetStructSize());

		if (Spec->GetAbility() == DefaultAbility)
			return true;
	}

	return false;
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

	static auto PredictionKeyStruct = FindObject<UStruct>(L"/Script/GameplayAbilities.PredictionKey");
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

		AbilitySystemComponent->GetActivatableAbilities()->MarkItemDirty(Spec);
	}
	else
	{
		// LOG_INFO(LogAbilities, "InternalServerTryActivateAbility. Activated {}", AbilityToActivate->GetName());
	}
}

FGameplayAbilitySpecHandle UAbilitySystemComponent::GiveAbilityEasy(UClass* AbilityClass, UObject* SourceObject, bool bDoNotRegive)
{
	if (!AbilityClass)
	{
		LOG_WARN(LogAbilities, "Invalid AbilityClass passed into GiveAbilityEasy!");
		return FGameplayAbilitySpecHandle();
	}

	// LOG_INFO(LogDev, "Making spec!");

	auto DefaultAbility = AbilityClass->CreateDefaultObject();

	if (!DefaultAbility)
		return FGameplayAbilitySpecHandle();

	if (bDoNotRegive && HasAbility(DefaultAbility))
		return FGameplayAbilitySpecHandle();

	auto NewSpec = MakeNewSpec((UClass*)DefaultAbility, SourceObject, true);

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

void UAbilitySystemComponent::RemoveActiveGameplayEffectBySourceEffect(UClass* GameplayEffect, UAbilitySystemComponent* InstigatorAbilitySystemComponent, int StacksToRemove)
{
	static auto RemoveActiveGameplayEffectBySourceEffectFn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.RemoveActiveGameplayEffectBySourceEffect");

	struct
	{
		UClass* GameplayEffect;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		UAbilitySystemComponent* InstigatorAbilitySystemComponent;                         // (Parm, ZeroConstructor, InstancedReference, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		int                                                StacksToRemove;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} UAbilitySystemComponent_RemoveActiveGameplayEffectBySourceEffect_Params{GameplayEffect, InstigatorAbilitySystemComponent, StacksToRemove};

	this->ProcessEvent(RemoveActiveGameplayEffectBySourceEffectFn, &UAbilitySystemComponent_RemoveActiveGameplayEffectBySourceEffect_Params);
}

void UAbilitySystemComponent::ClearAbility(const FGameplayAbilitySpecHandle& Handle)
{
	if (!Addresses::ClearAbility)
	{
		LOG_INFO(LogDev, "Invalid clear ability!");
		return;
	}

	static void (*ClearAbilityOriginal)(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAbilitySpecHandle& Handle) = decltype(ClearAbilityOriginal)(Addresses::ClearAbility);
	ClearAbilityOriginal(this, Handle);
}
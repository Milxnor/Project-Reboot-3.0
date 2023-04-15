#include "FortGadgetItemDefinition.h"
#include "FortAbilitySet.h"
#include "SoftObjectPath.h"
#include "FortPlayerStateAthena.h"
#include "addresses.h"

void UFortGadgetItemDefinition::UnequipGadgetData(AFortPlayerController* PlayerController, UFortItem* Item)
{
	static auto FortInventoryOwnerInterfaceClass = FindObject<UClass>("/Script/FortniteGame.FortInventoryOwnerInterface");
	__int64 (*RemoveGadgetDataOriginal)(UFortGadgetItemDefinition* a1, __int64 a2, UFortItem* a3) = decltype(RemoveGadgetDataOriginal)(Addresses::RemoveGadgetData);
	RemoveGadgetDataOriginal(this, __int64(PlayerController->GetInterfaceAddress(FortInventoryOwnerInterfaceClass)), Item);

	/* auto Pawn = PlayerController->GetMyFortPawn();

	if (Pawn)
	{
		static auto OriginalFootstepBankOffset = Pawn->GetOffset("OriginalFootstepBank");
		static auto FootstepBankOverrideOffset = Pawn->GetOffset("FootstepBankOverride");
		Pawn->Get(FootstepBankOverrideOffset) = Pawn->Get(OriginalFootstepBankOffset);

		static auto AnimBPOverrideOffset = Pawn->GetOffset("AnimBPOverride");
		static auto OriginalAnimBPOffset = Pawn->GetOffset("OriginalAnimBP");
		Pawn->Get(AnimBPOverrideOffset) = Pawn->Get(OriginalAnimBPOffset);
	}

	static auto AbilitySetOffset = this->GetOffset("AbilitySet");
	auto& AbilitySetSoft = this->Get<TSoftObjectPtr<UFortAbilitySet>>(AbilitySetOffset);

	auto StrongAbilitySet = AbilitySetSoft.Get(UFortAbilitySet::StaticClass(), true);

	if (StrongAbilitySet)
	{
		auto PlayerState = (AFortPlayerStateAthena*)PlayerController->GetPlayerState();
		auto ASC = PlayerState ? PlayerState->GetAbilitySystemComponent() : nullptr;

		if (ASC)
		{
			if (FGameplayEffectApplicationInfoHard::GetStruct())
			{
				auto AS_GrantedGameplayEffects = StrongAbilitySet->GetGrantedGameplayEffects();

				if (AS_GrantedGameplayEffects)
				{
					for (int i = 0; i < AS_GrantedGameplayEffects->Num(); i++)
					{
						ASC->RemoveActiveGameplayEffectBySourceEffect(AS_GrantedGameplayEffects->at(i, FGameplayEffectApplicationInfoHard::GetStructSize()).GameplayEffect, ASC, 1);
					}
				}
			}

			auto& ActivatableAbilitiesItems = ASC->GetActivatableAbilities()->GetItems();
			auto AS_GameplayAbilities = StrongAbilitySet->GetGameplayAbilities();

			for (int j = 0; j < AS_GameplayAbilities->Num(); j++)
			{
				auto CurrentDefaultAbility = AS_GameplayAbilities->at(j)->CreateDefaultObject();

				for (int i = 0; i < ActivatableAbilitiesItems.Num(); i++)
				{
					auto Spec = ActivatableAbilitiesItems.AtPtr(i, FGameplayAbilitySpec::GetStructSize());

					if (Spec->GetAbility() == CurrentDefaultAbility)
					{
						ASC->ClearAbility(Spec->GetHandle());
					}
				}
			}
		}
	}

	PlayerController->ApplyCosmeticLoadout();
	*/
}
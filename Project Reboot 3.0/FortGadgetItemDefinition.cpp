#include "FortGadgetItemDefinition.h"
#include "FortAbilitySet.h"
#include "SoftObjectPath.h"
#include "FortPlayerStateAthena.h"
#include "addresses.h"
#include "FortPlayerPawnAthena.h"
#include "FortPlayerControllerAthena.h"

void UFortGadgetItemDefinition::UnequipGadgetData(AFortPlayerController* PlayerController, UFortItem* Item)
{
	static auto FortInventoryOwnerInterfaceClass = FindObject<UClass>("/Script/FortniteGame.FortInventoryOwnerInterface");
	__int64 (*RemoveGadgetDataOriginal)(UFortGadgetItemDefinition* a1, __int64 a2, UFortItem* a3) = decltype(RemoveGadgetDataOriginal)(Addresses::RemoveGadgetData);
	RemoveGadgetDataOriginal(this, __int64(PlayerController->GetInterfaceAddress(FortInventoryOwnerInterfaceClass)), Item);

	if (auto CosmeticLoadoutPC = PlayerController->GetCosmeticLoadout())
	{
		if (auto CharacterToApply = CosmeticLoadoutPC->GetCharacter())
		{
			ApplyCID(Cast<AFortPlayerPawn>(PlayerController->GetMyFortPawn()), CharacterToApply); // idk why no automatic
		}
	}
}

void UFortGadgetItemDefinition::UpdateTrackedAttributesHook(UFortGadgetItemDefinition* GadgetItemDefinition)
{
	// LOG_INFO(LogDev, "UpdateTrackedAttributesHook Return: 0x{:x}", __int64(_ReturnAddress()) - __int64(GetModuleHandleW(0)));

	return;

	AFortPlayerState* PlayerState = nullptr; // how do we get it bro....
	UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent();

	if (GadgetItemDefinition->ShouldDestroyGadgetWhenTrackedAttributesIsZero())
	{
		bool bIsEveryTrackedAttributeZero = true;

		for (int i = 0; i < GadgetItemDefinition->GetTrackedAttributes().Num(); i++)
		{
			auto& CurrentTrackedAttribute = GadgetItemDefinition->GetTrackedAttributes().at(i);

			// LOG_INFO(LogDev, "[{}] TrackedAttribute Attribute Property Name {}", i, GadgetItemDefinition->GetTrackedAttributes().at(i).GetAttributePropertyName());
			// LOG_INFO(LogDev, "[{}] TrackedAttribute Attribute Name {}", i, GadgetItemDefinition->GetTrackedAttributes().at(i).GetAttributeName());
			// LOG_INFO(LogDev, "[{}] TrackedAttribute Attribute Owner {}", i, GadgetItemDefinition->GetTrackedAttributes().at(i).AttributeOwner->GetPathName());

			if (!ASC)
				break;

			int CurrentAttributeValue = -1;

			for (int i = 0; i < ASC->GetSpawnedAttributes().Num(); i++)
			{
				auto CurrentSpawnedAttribute = ASC->GetSpawnedAttributes().at(i);

				if (CurrentSpawnedAttribute->IsA(CurrentTrackedAttribute.AttributeOwner))
				{
					auto PropertyOffset = CurrentSpawnedAttribute->GetOffset(CurrentTrackedAttribute.GetAttributePropertyName());

					if (PropertyOffset != -1)
					{
						if (CurrentSpawnedAttribute->GetPtr<FFortGameplayAttributeData>(PropertyOffset)->GetCurrentValue() != 0)
						{
							bIsEveryTrackedAttributeZero = false;
							break;
						}
					}
				}
			}
		}

		if (bIsEveryTrackedAttributeZero)
		{
			// REMOVE THE ITEM

			static auto MulticastTriggerOnGadgetTrackedAttributeDestroyedFXFn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerStateZone.MulticastTriggerOnGadgetTrackedAttributeDestroyedFX");
			PlayerState->ProcessEvent(MulticastTriggerOnGadgetTrackedAttributeDestroyedFXFn, &GadgetItemDefinition);
		}
	}
}
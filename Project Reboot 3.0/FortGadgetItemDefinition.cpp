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
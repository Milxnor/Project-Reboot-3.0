#include "BuildingActor.h"

#include "FortWeapon.h"
#include "BuildingSMActor.h"
#include "FortPlayerControllerAthena.h"
#include "FortPawn.h"
#include "FortWeaponMeleeItemDefinition.h"
#include "CurveTable.h"
#include "DataTable.h"
#include "FortResourceItemDefinition.h"
#include "FortKismetLibrary.h"
#include "DataTableFunctionLibrary.h"
#include "FortGameStateAthena.h"
#include "GameplayStatics.h"
#include "BuildingContainer.h"
#include "FortPlayerController.h"

#include "Rotator.h"
#include "BuildingSMActor.h"
#include "FortGameModeAthena.h"

#include "FortPlayerState.h"
#include "BuildingWeapons.h"

#include "ActorComponent.h"
#include "FortPlayerStateAthena.h"
#include "globals.h"
#include "FortPlayerControllerAthena.h"
#include "FortLootPackage.h"
#include "FortPickup.h"
#include "FortPlayerPawn.h"
#include <memcury.h>
#include "KismetStringLibrary.h"
#include "FortGadgetItemDefinition.h"
#include "FortAbilitySet.h"
#include "vendingmachine.h"
#include "KismetSystemLibrary.h"
#include "gui.h"
#include "FortAthenaMutator_InventoryOverride.h"
#include "FortAthenaMutator_TDM.h"
#include <unordered_set>
#include "FortAthenaAIBotController.h"
#include "FortAthenaMutator_ItemDropOnDeath.h"
#include "FortAthenaMutator_GG.h"


void ABuildingActor::OnDamageServerHook(ABuildingActor* BuildingActor, float Damage, FGameplayTagContainer DamageTags,
	FVector Momentum, /* FHitResult */ __int64 HitInfo, APlayerController* InstigatedBy, AActor* DamageCauser,
	/* FGameplayEffectContextHandle */ __int64 EffectContext)
{
	// LOG_INFO(LogDev, "Befor3e");

	auto BuildingSMActor = Cast<ABuildingSMActor>(BuildingActor);
	if (auto Container = Cast<ABuildingContainer>(BuildingActor))
	{
		if ((BuildingSMActor->GetHealth() <= 0 || BuildingActor->GetHealth() <= 0) && !Container->IsAlreadySearched())
		{
			Container->SpawnLoot();
		}
	}
	auto AttachedBuildingActors = BuildingSMActor->GetAttachedBuildingActors();
	for (int i = 0; i < AttachedBuildingActors.Num(); ++i)
	{

		auto CurrentBuildingActor = AttachedBuildingActors.at(i);
		auto CurrentActor = Cast<ABuildingActor>(CurrentBuildingActor);
		if (BuildingSMActor->GetHealth() <= 0 || BuildingActor->GetHealth() <= 0)
		{
			if (auto Container = Cast<ABuildingContainer>(CurrentActor))
			{
				if (!Container->IsAlreadySearched()) 
				{
					Container->SpawnLoot();
				}
			}
		}
	}
	auto PlayerController = Cast<AFortPlayerControllerAthena>(InstigatedBy);
	// auto Pawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
	auto Weapon = Cast<AFortWeapon>(DamageCauser);

	if (!BuildingSMActor)
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	if (BuildingSMActor->IsDestroyed())
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	/*

	static auto LastDamageAmountOffset = BuildingSMActor->GetOffset("LastDamageAmount");
	static auto LastDamageHitOffset = BuildingSMActor->GetOffset("LastDamageHit", false) != -1 ? BuildingSMActor->GetOffset("LastDamageHit") : BuildingSMActor->GetOffset("LastDamageHitImpulseDir"); // idc

	const float PreviousLastDamageAmount = BuildingSMActor->Get<float>(LastDamageAmountOffset);
	const float PreviousLastDamageHit = BuildingSMActor->Get<float>(LastDamageHitOffset);
	const float CurrentBuildingHealth = BuildingActor->GetHealth();

	BuildingSMActor->Get<float>(LastDamageAmountOffset) = Damage;
	BuildingSMActor->Get<float>(LastDamageHitOffset) = CurrentBuildingHealth;

	*/

	if (!PlayerController || !Weapon)
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	// if (!Pawn)
		// return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	auto WorldInventory = PlayerController->GetWorldInventory();

	if (!WorldInventory)
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	auto WeaponData = Cast<UFortWeaponMeleeItemDefinition>(Weapon->GetWeaponData());

	if (!WeaponData)
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	UFortResourceItemDefinition* ItemDef = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingSMActor->GetResourceType());

	if (!ItemDef)
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	static auto BuildingResourceAmountOverrideOffset = BuildingSMActor->GetOffset("BuildingResourceAmountOverride");
	auto& BuildingResourceAmountOverride = BuildingSMActor->Get<FCurveTableRowHandle>(BuildingResourceAmountOverrideOffset);

	int ResourceCount = 0;


	if (BuildingResourceAmountOverride.RowName.IsValid())
	{
		// auto AssetManager = Cast<UFortAssetManager>(GEngine->AssetManager);
		//auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
		UCurveTable* CurveTable = nullptr; //GameState->GetCurrentPlaylist().BasePlaylist ? GameState->GetCurrentPlaylist().BasePlaylist->ResourceRates.Get() : nullptr;

		// LOG_INFO(LogDev, "Before1");

		if (!CurveTable)
			CurveTable = FindObject<UCurveTable>(L"/Game/Athena/Balance/DataTables/AthenaResourceRates.AthenaResourceRates");

		{
			// auto curveMap = ((UDataTable*)CurveTable)->GetRowMap();

			// LOG_INFO(LogDev, "Before {}", __int64(CurveTable));

			float Out = UDataTableFunctionLibrary::EvaluateCurveTableRow(CurveTable, BuildingResourceAmountOverride.RowName, 0.f);

			// LOG_INFO(LogDev, "Out: {}", Out);

			const float DamageThatWillAffect = /* PreviousLastDamageHit > 0 && Damage > PreviousLastDamageHit ? PreviousLastDamageHit : */ Damage;

			float skid = Out / (BuildingActor->GetMaxHealth() / DamageThatWillAffect);

			ResourceCount = round(skid);
		}
	}

	if (ResourceCount <= 0)
	{
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
	}

	bool bIsWeakspot = Damage == 100.0f;
	PlayerController->ClientReportDamagedResourceBuilding(BuildingSMActor, BuildingSMActor->GetResourceType(), ResourceCount, false, bIsWeakspot);

	bool bShouldUpdate = false;
	WorldInventory->AddItem(ItemDef, &bShouldUpdate, ResourceCount);

	if (bShouldUpdate)
		WorldInventory->Update();

	return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
}

UClass* ABuildingActor::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.BuildingActor");
	return Class;
}

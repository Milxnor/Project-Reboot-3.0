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

void ABuildingActor::OnDamageServerHook(ABuildingActor* BuildingActor, float Damage, FGameplayTagContainer DamageTags,
	FVector Momentum, /* FHitResult */ __int64 HitInfo, APlayerController* InstigatedBy, AActor* DamageCauser,
	/* FGameplayEffectContextHandle */ __int64 EffectContext)
{
	// LOG_INFO(LogDev, "Befor3e");

	auto BuildingSMActor = Cast<ABuildingSMActor>(BuildingActor);
	auto PlayerController = Cast<AFortPlayerControllerAthena>(InstigatedBy);
	auto Pawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
	auto Weapon = Cast<AFortWeapon>(DamageCauser);

	if (!BuildingSMActor || !PlayerController || !Pawn || !Weapon)
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	auto WeaponData = Cast<UFortWeaponMeleeItemDefinition>(Weapon->GetWeaponData());

	if (!WeaponData)
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
 
	if (BuildingSMActor->IsDestroyed())
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	auto ResourceCount = 0;
	UFortResourceItemDefinition* ItemDef = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingSMActor->GetResourceType());

	static auto BuildingResourceAmountOverrideOffset = BuildingSMActor->GetOffset("BuildingResourceAmountOverride");
	auto& BuildingResourceAmountOverride = BuildingSMActor->Get<FCurveTableRowHandle>(BuildingResourceAmountOverrideOffset);

	if (BuildingResourceAmountOverride.RowName.IsValid())
	{
		// auto AssetManager = Cast<UFortAssetManager>(GEngine->AssetManager);
		// auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameStateAthena);
		UCurveTable* CurveTable = nullptr; // GameState->CurrentPlaylistInfo.BasePlaylist ? GameState->CurrentPlaylistInfo.BasePlaylist->ResourceRates.Get() : nullptr;

		// LOG_INFO(LogDev, "Before1");

		if (!CurveTable)
			CurveTable = FindObject<UCurveTable>("/Game/Athena/Balance/DataTables/AthenaResourceRates.AthenaResourceRates");

		{
			// auto curveMap = ((UDataTable*)CurveTable)->GetRowMap();

			// LOG_INFO(LogDev, "Before {}", __int64(CurveTable));

			float Out = UDataTableFunctionLibrary::EvaluateCurveTableRow(CurveTable, BuildingResourceAmountOverride.RowName, 0.f);

			// LOG_INFO(LogDev, "Out: {}", Out);

			auto DamageThatWillAffect = Damage;

			float skid = Out / (BuildingSMActor->GetMaxHealth() / DamageThatWillAffect);

			ResourceCount = round(skid); // almost right
		}
	}

	if (!ItemDef || ResourceCount <= 0)
	{
		return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
		// return OnDamageServer(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
	}

	bool bIsWeakspot = Damage == 100.0f;
	PlayerController->ClientReportDamagedResourceBuilding(BuildingSMActor, BuildingSMActor->GetResourceType(), ResourceCount, false, bIsWeakspot);

	if (ResourceCount > 0)
	{
		bool bShouldUpdate = false;
		PlayerController->GetWorldInventory()->AddItem(ItemDef, &bShouldUpdate, ResourceCount);

		if (bShouldUpdate)
			PlayerController->GetWorldInventory()->Update();
	}

	return OnDamageServerOriginal(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
}

UClass* ABuildingActor::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.BuildingActor");
	return Class;
}

#include "GameModeBase.h"

#include "reboot.h"
#include "FortPlayerControllerAthena.h"

APawn* AGameModeBase::SpawnDefaultPawnForHook(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot)
{
	LOG_INFO(LogDev, "SpawnDefaultPawnFor: 0x{:x}!", __int64(_ReturnAddress()) - __int64(GetModuleHandleW(0)));

	static auto PawnClass = FindObject<UClass>("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
	GameMode->Get<UClass*>("DefaultPawnClass") = PawnClass;

	static auto fn = FindObject<UFunction>(L"/Script/Engine.GameModeBase.SpawnDefaultPawnAtTransform");

	struct { AController* NewPlayer; FTransform SpawnTransform; APawn* ReturnValue; } 
	AGameModeBase_SpawnDefaultPawnAtTransform_Params{NewPlayer, StartSpot->GetTransform()};

	GameMode->ProcessEvent(fn, &AGameModeBase_SpawnDefaultPawnAtTransform_Params);

	auto NewPlayerAsAthena = Cast<AFortPlayerControllerAthena>(NewPlayer);

	if (NewPlayerAsAthena)
	{
		auto WorldInventory = NewPlayerAsAthena->GetWorldInventory();

		auto CosmeticLoadoutPickaxe = NewPlayerAsAthena->GetCosmeticLoadout()->GetPickaxe();
		static auto WeaponDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.AthenaPickaxeItemDefinition", "WeaponDefinition");

		auto Pickaxe = CosmeticLoadoutPickaxe ? CosmeticLoadoutPickaxe->Get<UFortItemDefinition*>(WeaponDefinitionOffset)
			: FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");

		static UFortItemDefinition* EditToolItemDefinition = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/EditTool.EditTool");
		static UFortItemDefinition* PickaxeDefinition = Pickaxe;
		static UFortItemDefinition* BuildingItemData_Wall = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall");
		static UFortItemDefinition* BuildingItemData_Floor = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor");
		static UFortItemDefinition* BuildingItemData_Stair_W = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W");
		static UFortItemDefinition* BuildingItemData_RoofS = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS");
		static UFortItemDefinition* WoodItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");

		WorldInventory->AddItem(EditToolItemDefinition, nullptr);
		WorldInventory->AddItem(BuildingItemData_Wall, nullptr);
		WorldInventory->AddItem(BuildingItemData_Floor, nullptr);
		WorldInventory->AddItem(BuildingItemData_Stair_W, nullptr);
		WorldInventory->AddItem(BuildingItemData_RoofS, nullptr);
		WorldInventory->AddItem(PickaxeDefinition, nullptr);
		WorldInventory->AddItem(WoodItemData, nullptr, 100);

		WorldInventory->Update(true);
	}


	return AGameModeBase_SpawnDefaultPawnAtTransform_Params.ReturnValue;
}
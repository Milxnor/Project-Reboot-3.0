#include "GameModeBase.h"

#include "reboot.h"
#include "FortPlayerControllerAthena.h"
#include "FortGameModeAthena.h"
#include "FortLootPackage.h"
#include "FortAthenaMutator_GiveItemsAtGamePhaseStep.h"
#include "DataTableFunctionLibrary.h"
#include "FortAthenaMutator_GG.h"

UClass* AGameModeBase::GetDefaultPawnClassForController(AController* InController)
{
	static auto GetDefaultPawnClassForControllerFn = FindObject<UFunction>("/Script/Engine.GameModeBase.GetDefaultPawnClassForController");
	struct
	{
		AController* InController;                                             // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		UClass* ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} AGameModeBase_GetDefaultPawnClassForController_Params{InController};

	this->ProcessEvent(GetDefaultPawnClassForControllerFn, &AGameModeBase_GetDefaultPawnClassForController_Params);

	return AGameModeBase_GetDefaultPawnClassForController_Params.ReturnValue;
}

APawn* AGameModeBase::SpawnDefaultPawnForHook(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot)
{
	LOG_INFO(LogDev, "SpawnDefaultPawnFor: 0x{:x}!", __int64(_ReturnAddress()) - __int64(GetModuleHandleW(0)));

	// auto PawnClass = GameMode->GetDefaultPawnClassForController(NewPlayer);
	// LOG_INFO(LogDev, "PawnClass: {}", PawnClass->GetFullName());

	static auto PawnClass = FindObject<UClass>("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
	GameMode->Get<UClass*>("DefaultPawnClass") = PawnClass;

	constexpr bool bUseSpawnActor = false;

	static auto fn = FindObject<UFunction>(L"/Script/Engine.GameModeBase.SpawnDefaultPawnAtTransform");

	FTransform SpawnTransform = StartSpot->GetTransform();
	APawn* NewPawn = nullptr;

	FActorSpawnParameters SpawnParameters{};
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if constexpr (bUseSpawnActor)
	{
NewPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnParameters);
	}
	else
	{
	struct { AController* NewPlayer; FTransform SpawnTransform; APawn* ReturnValue; }
	AGameModeBase_SpawnDefaultPawnAtTransform_Params{ NewPlayer, SpawnTransform };

	GameMode->ProcessEvent(fn, &AGameModeBase_SpawnDefaultPawnAtTransform_Params);

	NewPawn = AGameModeBase_SpawnDefaultPawnAtTransform_Params.ReturnValue;
	}

	if (!NewPawn)
		return nullptr;

	bool bIsRespawning = false;

	if (!bIsRespawning)
	{
		auto NewPlayerAsAthena = Cast<AFortPlayerControllerAthena>(NewPlayer);

		auto GameState = ((AFortGameModeAthena*)GameMode)->GetGameStateAthena();

		GET_PLAYLIST(GameState);

		if (CurrentPlaylist)
		{
			CurrentPlaylist->ApplyModifiersToActor(NewPlayerAsAthena->GetPlayerState()); // We need to move this!
		}

		/* if (Fortnite_Version >= 18)
		{
			static auto StormEffectClass = FindObject<UClass>("/Game/Athena/SafeZone/GE_OutsideSafeZoneDamage.GE_OutsideSafeZoneDamage_C");
			auto PlayerState = NewPlayerAsAthena->GetPlayerStateAthena();
			PlayerState->GetAbilitySystemComponent()->RemoveActiveGameplayEffectBySourceEffect(StormEffectClass, 1, PlayerState->GetAbilitySystemComponent());
		} */

		if (NewPlayerAsAthena)
		{
			auto WorldInventory = NewPlayerAsAthena->GetWorldInventory();

			if (!WorldInventory)
				return NewPawn;

			if (!WorldInventory->GetPickaxeInstance())
			{
				auto CosmeticLoadout = NewPlayerAsAthena->GetCosmeticLoadout();
				// LOG_INFO(LogDev, "CosmeticLoadout: {}", __int64(CosmeticLoadout));
				auto CosmeticLoadoutPickaxe = CosmeticLoadout ? CosmeticLoadout->GetPickaxe() : nullptr;
				// LOG_INFO(LogDev, "CosmeticLoadoutPickaxe: {}", __int64(CosmeticLoadoutPickaxe));
				// LOG_INFO(LogDev, "CosmeticLoadoutPickaxe Name: {}", CosmeticLoadoutPickaxe ? CosmeticLoadoutPickaxe->GetFullName() : "InvalidObject");
				static auto WeaponDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.AthenaPickaxeItemDefinition", "WeaponDefinition");

				auto PickaxeDefinition = CosmeticLoadoutPickaxe ? CosmeticLoadoutPickaxe->Get<UFortItemDefinition*>(WeaponDefinitionOffset)
					: FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");

				static UFortItemDefinition* EditToolItemDefinition = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/EditTool.EditTool");
				static UFortItemDefinition* BuildingItemData_Wall = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall");
				static UFortItemDefinition* BuildingItemData_Floor = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor");
				static UFortItemDefinition* BuildingItemData_Stair_W = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W");
				static UFortItemDefinition* BuildingItemData_RoofS = FindObject<UFortItemDefinition>(L"/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS");
				static UFortItemDefinition* WoodItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
				static UFortItemDefinition* DamageTrap = FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Traps/TID_ContextTrap_Athena.TID_ContextTrap_Athena");

				WorldInventory->AddItem(PickaxeDefinition, nullptr);
				WorldInventory->AddItem(EditToolItemDefinition, nullptr);
				WorldInventory->AddItem(BuildingItemData_Wall, nullptr);
				WorldInventory->AddItem(BuildingItemData_Floor, nullptr);
				WorldInventory->AddItem(BuildingItemData_Stair_W, nullptr);
				WorldInventory->AddItem(BuildingItemData_RoofS, nullptr);
				// WorldInventory->AddItem(WoodItemData, nullptr, 100);
				// WorldInventory->AddItem(DamageTrap, nullptr);
				// WorldInventory->AddItem(FindObject<UFortItemDefinition>(L"/ParallelGameplay/Items/WestSausage/WID_WestSausage_Parallel.WID_WestSausage_Parallel"), nullptr, 1, 1000);
				// WorldInventory->AddItem(FindObject<UFortItemDefinition>(L"/Game/Athena/Items/Consumables/HappyGhost/WID_Athena_HappyGhost.WID_Athena_HappyGhost"), nullptr);

				/* if (Globals::bLateGame)
				{
					auto SpawnIslandTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaFloorLoot_Warmup");

					for (int i = 0; i < 5; i++)
					{
						auto LootDrops = PickLootDrops(SpawnIslandTierGroup);

						for (auto& LootDrop : LootDrops)
						{
							WorldInventory->AddItem(LootDrop.ItemDefinition, nullptr, LootDrop.Count, LootDrop.LoadedAmmo);
						}
					}
				} */

				WorldInventory->Update();
			}
		}
	}

	return NewPawn;
}
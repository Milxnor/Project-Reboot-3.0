#include "GameModeBase.h"

#include "reboot.h"
#include "FortPlayerControllerAthena.h"
#include "FortGameModeAthena.h"
#include "FortLootPackage.h"
#include "FortAthenaMutator_GiveItemsAtGamePhaseStep.h"
#include "DataTableFunctionLibrary.h"
#include "FortAthenaMutator_GG.h"
#include "FortAthenaMutator_InventoryOverride.h"

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

void AGameModeBase::ChangeName(AController* Controller, const FString& NewName, bool bNameChange)
{
	static auto ChangeNameFn = FindObject<UFunction>("/Script/Engine.GameModeBase.ChangeName");

	struct
	{
		AController* Controller;                                               // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		struct FString                                     NewName;                                                  // (Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		bool                                               bNameChange;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} AGameModeBase_ChangeName_Params{ Controller, NewName, bNameChange };

	this->ProcessEvent(ChangeNameFn, &AGameModeBase_ChangeName_Params);
}

AActor* AGameModeBase::K2_FindPlayerStart(AController* Player, FString IncomingName)
{
	static auto K2_FindPlayerStartFn = FindObject<UFunction>("/Script/Engine.GameModeBase.K2_FindPlayerStart");

	struct
	{
		AController* Player;                                                   // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FString                                     IncomingName;                                             // (Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		AActor* ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} AGameModeBase_K2_FindPlayerStart_Params{ Player, IncomingName };

	this->ProcessEvent(K2_FindPlayerStartFn, &AGameModeBase_K2_FindPlayerStart_Params);
	
	return AGameModeBase_K2_FindPlayerStart_Params.ReturnValue;
}

APawn* AGameModeBase::SpawnDefaultPawnForHook(AGameModeBase* GameMode, AController* NewPlayer, AActor* StartSpot)
{
	auto NewPlayerAsAthena = Cast<AFortPlayerControllerAthena>(NewPlayer);

	if (!NewPlayerAsAthena)
		return nullptr; // return original?

	auto PlayerStateAthena = NewPlayerAsAthena->GetPlayerStateAthena();

	if (!PlayerStateAthena)
		return nullptr; // return original?

	static auto PawnClass = FindObject<UClass>("/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
	static auto DefaultPawnClassOffset = GameMode->GetOffset("DefaultPawnClass");
	GameMode->Get<UClass*>(DefaultPawnClassOffset) = PawnClass;

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

	static auto RespawnDataOffset = PlayerStateAthena->GetOffset("RespawnData", false);

	if (RespawnDataOffset != -1)
	{
		static auto bServerIsReadyOffset = FindOffsetStruct("/Script/FortniteGame.FortRespawnData", "bServerIsReady");
		static auto bRespawnDataAvailableOffset = FindOffsetStruct("/Script/FortniteGame.FortRespawnData", "bRespawnDataAvailable");

		auto RespawnDataPtr = PlayerStateAthena->GetPtr<__int64>(RespawnDataOffset);

		if (*(bool*)(__int64(RespawnDataPtr) + bServerIsReadyOffset) && *(bool*)(__int64(RespawnDataPtr) + bRespawnDataAvailableOffset)) // && GameState->IsRespawningAllowed(PlayerState);
		{
			// SpawnTransform.Translation = PlayerState->RespawnData.RespawnLocation;
			// SpawnTransform.Rotation = Quaternion(PlayerState->RespawnData.RespawnRotation);

			bIsRespawning = true;
		}
	}

	auto ASC = PlayerStateAthena->GetAbilitySystemComponent();
	auto GameState = ((AFortGameModeAthena*)GameMode)->GetGameStateAthena();

	GET_PLAYLIST(GameState);

	if (CurrentPlaylist) // Apply gameplay effects from playlist // We need to move this maybe?
	{
		CurrentPlaylist->ApplyModifiersToActor(PlayerStateAthena);
	}

	auto PlayerAbilitySet = GetPlayerAbilitySet(); // Apply default gameplay effects // We need to move maybe?

	if (PlayerAbilitySet && ASC)
	{
		PlayerAbilitySet->ApplyGrantedGameplayAffectsToAbilitySystem(ASC);
	}

	if (!bIsRespawning)
	{
		auto WorldInventory = NewPlayerAsAthena->GetWorldInventory();

		if (!WorldInventory)
			return NewPawn;

		if (!WorldInventory->GetPickaxeInstance())
		{
			// TODO Check Playlist->bRequirePickaxeInStartingInventory

			auto& StartingItems = ((AFortGameModeAthena*)GameMode)->GetStartingItems();

			NewPlayerAsAthena->AddPickaxeToInventory();

			for (int i = 0; i < StartingItems.Num(); i++)
			{
				auto& StartingItem = StartingItems.at(i);

				WorldInventory->AddItem(StartingItem.GetItem(), nullptr, StartingItem.GetCount());
			}

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

			auto AddInventoryOverrideTeamLoadouts = [&](AFortAthenaMutator* Mutator)
			{
				if (auto InventoryOverride = Cast<AFortAthenaMutator_InventoryOverride>(Mutator))
				{
					auto TeamIndex = PlayerStateAthena->GetTeamIndex();
					auto LoadoutTeam = InventoryOverride->GetLoadoutTeamForTeamIndex(TeamIndex);

					if (LoadoutTeam.UpdateOverrideType == EAthenaInventorySpawnOverride::Always)
					{
						auto LoadoutContainer = InventoryOverride->GetLoadoutContainerForTeamIndex(TeamIndex);

						for (int i = 0; i < LoadoutContainer.Loadout.Num(); i++)
						{
							auto& ItemAndCount = LoadoutContainer.Loadout.at(i);
							WorldInventory->AddItem(ItemAndCount.GetItem(), nullptr, ItemAndCount.GetCount());
						}
					}
				}
			};

			LoopMutators(AddInventoryOverrideTeamLoadouts);

			WorldInventory->Update();
		}
	}
	else
	{
		auto DeathInfo = (void*)(__int64(PlayerStateAthena) + MemberOffsets::FortPlayerStateAthena::DeathInfo);

		static auto DeathInfoStruct = FindObject<UStruct>(L"/Script/FortniteGame.DeathInfo");
		static auto DeathInfoStructSize = DeathInfoStruct->GetPropertiesSize();
		RtlSecureZeroMemory(DeathInfo, DeathInfoStructSize);
	}

	return NewPawn;
}
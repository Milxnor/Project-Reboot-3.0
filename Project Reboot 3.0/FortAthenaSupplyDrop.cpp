#include "FortAthenaSupplyDrop.h"
#include "FortGameStateAthena.h"

FVector AFortAthenaSupplyDrop::FindGroundLocationAt(FVector InLocation)
{
	static auto FindGroundLocationAtFn = FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaSupplyDrop.FindGroundLocationAt");

	struct
	{
		FVector                                     InLocation;                                               // (ConstParm, Parm, OutParm, ZeroConstructor, ReferenceParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		FVector                                     ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} AFortAthenaSupplyDrop_FindGroundLocationAt_Params{ InLocation };

	this->ProcessEvent(FindGroundLocationAtFn, &AFortAthenaSupplyDrop_FindGroundLocationAt_Params);

	return AFortAthenaSupplyDrop_FindGroundLocationAt_Params.ReturnValue;
}

AFortPickup* AFortAthenaSupplyDrop::SpawnPickupFromItemEntryHook(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
	LOG_INFO(LogDev, __FUNCTION__);
	return SpawnPickupFromItemEntryOriginal(Context, Stack, Ret);
}

AFortPickup* AFortAthenaSupplyDrop::SpawnGameModePickupHook(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
	UFortWorldItemDefinition* ItemDefinition = nullptr;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	UClass* PickupClass = nullptr;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                NumberToSpawn;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	AFortPawn* TriggeringPawn = nullptr;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     Position;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     Direction;

	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&PickupClass);
	Stack.StepCompiledIn(&NumberToSpawn);
	Stack.StepCompiledIn(&TriggeringPawn);
	Stack.StepCompiledIn(&Position);
	Stack.StepCompiledIn(&Direction);

	SpawnGameModePickupOriginal(Context, Stack, Ret);

	if (!ItemDefinition || !PickupClass)
		return nullptr;

	LOG_INFO(LogDev, "Spawning GameModePickup with ItemDefinition: {}", ItemDefinition->GetFullName());

	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	PickupCreateData CreateData;
	CreateData.ItemEntry = FFortItemEntry::MakeItemEntry(ItemDefinition, NumberToSpawn, -1, MAX_DURABILITY, ItemDefinition->GetFinalLevel(GameState->GetWorldLevel()));
	CreateData.SpawnLocation = Position;
	CreateData.PawnOwner = TriggeringPawn;
	CreateData.Source = EFortPickupSpawnSource::GetSupplyDropValue();
	CreateData.OverrideClass = PickupClass;
	CreateData.bShouldFreeItemEntryWhenDeconstructed = true;

	*Ret = AFortPickup::SpawnPickup(CreateData);
	return *Ret;
}

AFortPickup* AFortAthenaSupplyDrop::SpawnPickupHook(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
	UFortWorldItemDefinition* ItemDefinition = nullptr;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                NumberToSpawn;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	AFortPawn* TriggeringPawn = nullptr;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     Position;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     Direction;                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&NumberToSpawn);
	Stack.StepCompiledIn(&TriggeringPawn);
	Stack.StepCompiledIn(&Position);
	Stack.StepCompiledIn(&Direction);

	SpawnPickupOriginal(Context, Stack, Ret);

	if (!ItemDefinition)
		return nullptr;

	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

	PickupCreateData CreateData;
	CreateData.ItemEntry = FFortItemEntry::MakeItemEntry(ItemDefinition, NumberToSpawn, -1, MAX_DURABILITY, ItemDefinition->GetFinalLevel(GameState->GetWorldLevel()));
	CreateData.SpawnLocation = Position;
	CreateData.PawnOwner = TriggeringPawn;
	CreateData.Source = EFortPickupSpawnSource::GetSupplyDropValue();
	CreateData.bShouldFreeItemEntryWhenDeconstructed = true;

	*Ret = AFortPickup::SpawnPickup(CreateData);
	return *Ret;
}


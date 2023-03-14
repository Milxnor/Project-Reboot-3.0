#include "FortAthenaSupplyDrop.h"

AFortPickup* AFortAthenaSupplyDrop::SpawnPickupHook(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
	UFortWorldItemDefinition* ItemDefinition = nullptr;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	int                                                NumberToSpawn;                                            // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	AFortPawn* TriggeringPawn = nullptr;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     Position;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FVector                                     Direction;                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

	Stack.Step(Stack.Object, &ItemDefinition);
	Stack.Step(Stack.Object, &NumberToSpawn);
	Stack.Step(Stack.Object, &TriggeringPawn);
	Stack.Step(Stack.Object, &Position);
	Stack.Step(Stack.Object, &Direction);

	SpawnPickupOriginal(Context, Stack, Ret);

	if (!ItemDefinition)
		return nullptr;

	*Ret = AFortPickup::SpawnPickup(ItemDefinition, Position, NumberToSpawn, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::SupplyDrop, -1, TriggeringPawn);
	return *Ret;
}
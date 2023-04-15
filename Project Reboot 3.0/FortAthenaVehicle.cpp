#include "FortAthenaVehicle.h"

UFortWeaponItemDefinition* AFortAthenaVehicle::GetVehicleWeaponForSeat(int SeatIdx)
{
	static auto GetSeatWeaponComponentFn = FindObject<UFunction>("/Script/FortniteGame.FortAthenaVehicle.GetSeatWeaponComponent");

	UFortWeaponItemDefinition* VehicleWeaponDefinition = nullptr;

	LOG_INFO(LogDev, "SeatIndex: {}", SeatIdx);

	UObject* WeaponComponent = nullptr;

	if (GetSeatWeaponComponentFn)
	{
		struct { int SeatIndex; UObject* ReturnValue; } AFortAthenaVehicle_GetSeatWeaponComponent_Params{};

		this->ProcessEvent(GetSeatWeaponComponentFn, &AFortAthenaVehicle_GetSeatWeaponComponent_Params);

		WeaponComponent = AFortAthenaVehicle_GetSeatWeaponComponent_Params.ReturnValue;

		if (!WeaponComponent)
			return VehicleWeaponDefinition;

		static auto WeaponSeatDefinitionStructSize = FindObject<UClass>("/Script/FortniteGame.WeaponSeatDefinition")->GetPropertiesSize();
		static auto VehicleWeaponOffset = FindOffsetStruct("/Script/FortniteGame.WeaponSeatDefinition", "VehicleWeapon");
		static auto SeatIndexOffset = FindOffsetStruct("/Script/FortniteGame.WeaponSeatDefinition", "SeatIndex");
		static auto WeaponSeatDefinitionsOffset = WeaponComponent->GetOffset("WeaponSeatDefinitions");
		auto& WeaponSeatDefinitions = WeaponComponent->Get<TArray<__int64>>(WeaponSeatDefinitionsOffset);

		for (int i = 0; i < WeaponSeatDefinitions.Num(); i++)
		{
			auto WeaponSeat = WeaponSeatDefinitions.AtPtr(i, WeaponSeatDefinitionStructSize);

			if (*(int*)(__int64(WeaponSeat) + SeatIndexOffset) != SeatIdx)
				continue;

			VehicleWeaponDefinition = *(UFortWeaponItemDefinition**)(__int64(WeaponSeat) + VehicleWeaponOffset);

			break;
		}
	}
	else
	{
		static auto FerretWeaponItemDefinition = FindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Ferret_Weapon.Ferret_Weapon");
		static auto OctopusWeaponItemDefinition = FindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Vehicles/WID_Octopus_Weapon.WID_Octopus_Weapon");
		static auto InCannonWeaponItemDefinition = FindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Vehicles/ShipCannon_Weapon_InCannon.ShipCannon_Weapon_InCannon");
		static auto CannonWeaponItemDefinition = FindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/Vehicles/ShipCannon_Weapon.ShipCannon_Weapon");
		static auto TurretWeaponItemDefinition = FindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Traps/MountedTurret/MountedTurret_Weapon.MountedTurret_Weapon");

		auto ReceivingActorName = this->GetName();

		if (SeatIdx == 0)
		{
			if (ReceivingActorName.contains("Ferret")) // plane
			{
				VehicleWeaponDefinition = FerretWeaponItemDefinition;
			}
		}

		if (ReceivingActorName.contains("Octopus")) // baller
		{
			VehicleWeaponDefinition = OctopusWeaponItemDefinition;
		}

		else if (ReceivingActorName.contains("Cannon")) // cannon
		{
			VehicleWeaponDefinition = SeatIdx == 1 ? InCannonWeaponItemDefinition : CannonWeaponItemDefinition;
		}

		else if (ReceivingActorName.contains("MountedTurret"))
		{
			VehicleWeaponDefinition = TurretWeaponItemDefinition;
		}
	}

	return VehicleWeaponDefinition;
}
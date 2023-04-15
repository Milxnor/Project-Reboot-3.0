#pragma once

#include "Actor.h"

#include "FortAthenaVehicle.h"
#include "reboot.h"

class AFortAthenaSKPushCannon : public AFortAthenaVehicle // : public AFortAthenaSKPushVehicle
{
public:
	bool IsPushCannonBP()
	{
		static auto PushCannonBP = FindObject<UClass>("/Game/Athena/DrivableVehicles/PushCannon.PushCannon_C"); // We should loadobject it.
		return this->IsA(PushCannonBP);
	}

	void MultiCastPushCannonLaunchedPlayer()
	{
		static auto MultiCastPushCannonLaunchedPlayerFn = FindObject<UFunction>("/Script/FortniteGame.FortAthenaSKPushCannon.MultiCastPushCannonLaunchedPlayer");
		this->ProcessEvent(MultiCastPushCannonLaunchedPlayerFn);
	}

	void OnLaunchPawn(AFortPlayerPawn* Pawn, FVector LaunchDir)
	{
		static auto OnLaunchPawnFn = IsPushCannonBP() ? FindObject<UFunction>("/Game/Athena/DrivableVehicles/PushCannon.PushCannon_C.OnLaunchPawn") : FindObject<UFunction>("/Script/FortniteGame.FortAthenaSKPushCannon.OnLaunchPawn");
		struct
		{
			AFortPlayerPawn* Pawn;                                                     // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			struct FVector                                     LaunchDir;                                                // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		} AFortAthenaSKPushCannon_OnLaunchPawn_Params{ Pawn, LaunchDir };

		this->ProcessEvent(OnLaunchPawnFn, &AFortAthenaSKPushCannon_OnLaunchPawn_Params);
	}

	void OnPreLaunchPawn(AFortPlayerPawn* Pawn, FVector LaunchDir)
	{
		static auto OnPreLaunchPawnFn = IsPushCannonBP() ? FindObject<UFunction>("/Game/Athena/DrivableVehicles/PushCannon.PushCannon_C.OnPreLaunchPawn") : FindObject<UFunction>("/Script/FortniteGame.FortAthenaSKPushCannon.OnPreLaunchPawn");
		struct
		{
			AFortPlayerPawn* Pawn;                                                     // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FVector                                     LaunchDir;                                                // (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		} AFortAthenaSKPushCannon_OnPreLaunchPawn_Params{ Pawn, LaunchDir };

		this->ProcessEvent(OnPreLaunchPawnFn, &AFortAthenaSKPushCannon_OnPreLaunchPawn_Params);
	}

	void ShootPawnOut(const FVector& LaunchDir)
	{
		auto PawnToShoot = this->GetPawnAtSeat(1);

		LOG_INFO(LogDev, "PawnToShoot: {}", __int64(PawnToShoot));

		if (!PawnToShoot)
			return;

		this->OnPreLaunchPawn(PawnToShoot, LaunchDir);
		PawnToShoot->ServerOnExitVehicle(ETryExitVehicleBehavior::ForceAlways);
		this->OnLaunchPawn(PawnToShoot, LaunchDir);
		this->MultiCastPushCannonLaunchedPlayer();
	}

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>("/Script/FortniteGame.FortAthenaSKPushCannon");
		return Class;
	}
};
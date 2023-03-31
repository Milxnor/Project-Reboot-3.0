#include "FortPlayerPawn.h"
#include <memcury.h>

void AFortPlayerPawn::ServerChoosePart(EFortCustomPartType Part, UObject* ChosenCharacterPart)
{
	static auto fn = FindObject<UFunction>("/Script/FortniteGame.FortPlayerPawn.ServerChoosePart");

	struct
	{
		EFortCustomPartType                   Part;                                                     // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		UObject* ChosenCharacterPart;                                      // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} AFortPlayerPawn_ServerChoosePart_Params{Part, ChosenCharacterPart};

	this->ProcessEvent(fn, &AFortPlayerPawn_ServerChoosePart_Params);
}

void AFortPlayerPawn::ForceLaunchPlayerZiplinine() // Thanks android
{
	float ZiplineJumpDampening = -0.5f;
	float ZiplineJumpStrength = 1500.f;

	static auto CharacterMovementOffset = GetOffset("CharacterMovement");
	auto CharacterMovement = this->Get(CharacterMovementOffset);

	static auto VelocityOffset = CharacterMovement->GetOffset("Velocity");
	auto& v23 = CharacterMovement->Get<FVector>(VelocityOffset);
	//v23.X = abs(v23.X);
	//v23.Y = abs(v23.Y);

	FVector v21 = { -750, -750, ZiplineJumpStrength };

	if (ZiplineJumpDampening * v23.X >= -750.f)
		v21.X = fminf(ZiplineJumpDampening * v23.X, 750);

	if (ZiplineJumpDampening * v23.Y >= -750.f)
		v21.Y = fminf(ZiplineJumpDampening * v23.Y, 750);

	// todo check if in vehicle

	static auto LaunchCharacterFn = FindObject<UFunction>("/Script/Engine.Character.LaunchCharacter");

	struct
	{
		FVector                                     LaunchVelocity;                                           // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		bool                                               bXYOverride;                                              // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		bool                                               bZOverride;                                               // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	} ACharacter_LaunchCharacter_Params{ v21, false, false };
	ProcessEvent(LaunchCharacterFn, &ACharacter_LaunchCharacter_Params);
}

void AFortPlayerPawn::ServerSendZiplineStateHook(AFortPlayerPawn* Pawn, FZiplinePawnState InZiplineState)
{
	static auto ZiplineStateOffset = Pawn->GetOffset("ZiplineState");

	auto PawnZiplineState = Pawn->GetPtr<__int64>(ZiplineStateOffset);

	static auto AuthoritativeValueOffset = FindOffsetStruct("/Script/FortniteGame.ZiplinePawnState", "AuthoritativeValue");

	if (*(int*)(__int64(&InZiplineState) + AuthoritativeValueOffset) > *(int*)(__int64(PawnZiplineState) + AuthoritativeValueOffset))
	{
		static auto ZiplinePawnStateStruct = FindObject<UStruct>("/Script/FortniteGame.ZiplinePawnState");
		static auto ZiplinePawnStateSize = ZiplinePawnStateStruct->GetPropertiesSize();

		CopyStruct(PawnZiplineState, &InZiplineState, ZiplinePawnStateSize);

		static auto bIsZipliningOffset = FindOffsetStruct("/Script/FortniteGame.ZiplinePawnState", "bIsZiplining");
		static auto bJumpedOffset = FindOffsetStruct("/Script/FortniteGame.ZiplinePawnState", "bJumped");

		if (!(*(bool*)(__int64(PawnZiplineState) + bIsZipliningOffset)))
		{
			if ((*(bool*)(__int64(PawnZiplineState) + bJumpedOffset)))
			{
				Pawn->ForceLaunchPlayerZiplinine();
			}
		}
	}

	static void (*OnRep_ZiplineState)(AFortPlayerPawn* Pawn) = decltype(OnRep_ZiplineState)(Addresses::OnRep_ZiplineState);

	if (OnRep_ZiplineState)
		OnRep_ZiplineState(Pawn);
}

void AFortPlayerPawn::ServerHandlePickupHook(AFortPlayerPawn* Pawn, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound)
{
	if (!Pickup)
		return;

	static auto bPickedUpOffset = Pickup->GetOffset("bPickedUp");

	if (Pickup->Get<bool>(bPickedUpOffset))
	{
		LOG_INFO(LogDev, "Trying to pickup picked up weapon?");
		return;
	}

	static auto IncomingPickupsOffset = Pawn->GetOffset("IncomingPickups");
	Pawn->Get<TArray<AFortPickup*>>(IncomingPickupsOffset).Add(Pickup);

	auto PickupLocationData = Pickup->GetPickupLocationData();

	PickupLocationData->GetPickupTarget() = Pawn;
	PickupLocationData->GetFlyTime() = 0.40f;
	PickupLocationData->GetItemOwner() = Pawn;
	PickupLocationData->GetStartDirection() = InStartDirection;
	PickupLocationData->GetPickupGuid() = Pawn->GetCurrentWeapon() ? Pawn->GetCurrentWeapon()->GetItemEntryGuid() : FGuid();

	static auto OnRep_PickupLocationDataFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.OnRep_PickupLocationData");
	Pickup->ProcessEvent(OnRep_PickupLocationDataFn);

	Pickup->Get<bool>(bPickedUpOffset) = true;

	static auto OnRep_bPickedUpFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.OnRep_bPickedUp");
	Pickup->ProcessEvent(OnRep_bPickedUpFn);
}

void AFortPlayerPawn::ServerHandlePickupInfoHook(AFortPlayerPawn* Pawn, AFortPickup* Pickup, __int64 Params)
{
	LOG_INFO(LogDev, "ServerHandlePickupInfo!");
	return ServerHandlePickupHook(Pawn, Pickup, 0.40f, FVector(), false);
}

UClass* AFortPlayerPawn::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/FortniteGame.FortPlayerPawn");
	return Class;
}
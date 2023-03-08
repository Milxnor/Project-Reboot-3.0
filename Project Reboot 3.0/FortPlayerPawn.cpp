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

void AFortPlayerPawn::ServerSendZiplineStateHook(AFortPlayerPawn* Pawn, FZiplinePawnState InZiplineState)
{
	static auto ZiplineStateOffset = Pawn->GetOffset("ZiplineState");

	auto PawnZiplineState = Pawn->GetPtr<__int64>(ZiplineStateOffset);

	static auto AuthoritativeValueOffset = FindOffsetStruct("/Script/FortniteGame.ZiplinePawnState", "AuthoritativeValue");

	if (*(int*)(__int64(&InZiplineState) + AuthoritativeValueOffset) > *(int*)(__int64(PawnZiplineState) + AuthoritativeValueOffset))
	{
		static auto ZiplinePawnStateClass = FindObject<UStruct>("/Script/FortniteGame.ZiplinePawnState");
		static auto ZiplinePawnStateSize = ZiplinePawnStateClass->GetPropertiesSize();

		CopyStruct(PawnZiplineState, &InZiplineState, ZiplinePawnStateSize);
	}

	static bool bFoundFunc = false;

	static void (*OnRep_ZiplineState)(AFortPlayerPawn* Pawn);

	if (!bFoundFunc)
	{
		bFoundFunc = true;

		static auto Addrr = Memcury::Scanner::FindStringRef(L"ZIPLINES!! Role(%s)  AFortPlayerPawn::OnRep_ZiplineState ZiplineState.bIsZiplining=%d").Get();

		std::cout << "Addrr: " << Addrr << '\n';

		if (Addrr)
		{
			for (int i = 600; i >= 0; i--)
			{
				// LOG("[{}] 0x{:x} 0x{:x}", i, (int)*(uint8_t*)Addr - i, (int)*(uint8_t*)(Addr - i), (int)*(uint8_t*)(Addr - i + 1));

				if (*(uint8_t*)(uint8_t*)(Addrr - i) == 0x40 && *(uint8_t*)(uint8_t*)(Addrr - i + 1) == 0x53)
				{
					OnRep_ZiplineState = decltype(OnRep_ZiplineState)(Addrr - i);
				}
			}
		}

		// LOG_INFO(LogDev, "OnRep_ZiplineState: 0x{:x}\n", (uintptr_t)OnRep_ZiplineState - __int64(GetModuleHandleW(0)));
	}

	if (OnRep_ZiplineState)
		OnRep_ZiplineState(Pawn);
}

void AFortPlayerPawn::ServerHandlePickupHook(AFortPlayerPawn* Pawn, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound)
{
	if (!Pickup)
		return;

	static auto bPickedUpOffset = Pickup->GetOffset("bPickedUp");

	if (Pickup->Get<bool>(bPickedUpOffset))
		return;

	static auto IncomingPickupsOffset = Pawn->GetOffset("IncomingPickups");
	static auto PickupLocationDataOffset = Pickup->GetOffset("PickupLocationData");
	auto PickupLocationData = Pickup->GetPtr<__int64>(PickupLocationDataOffset);

	Pawn->Get<TArray<AFortPickup*>>(IncomingPickupsOffset).Add(Pickup);

	static auto PickupTargetOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "PickupTarget");
	static auto FlyTimeOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "FlyTime");
	static auto ItemOwnerOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "ItemOwner");
	static auto StartDirectionOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "StartDirection");
	static auto PickupGuidOffset = FindOffsetStruct("/Script/FortniteGame.FortPickupLocationData", "PickupGuid");

	*(AFortPawn**)(__int64(PickupLocationData) + PickupTargetOffset) = Pawn;
	*(float*)(__int64(PickupLocationData) + FlyTimeOffset) = 0.40f;
	*(AFortPawn**)(__int64(PickupLocationData) + ItemOwnerOffset) = Pawn;
	*(FVector*)(__int64(PickupLocationData) + StartDirectionOffset) = InStartDirection;
	*(FGuid*)(__int64(PickupLocationData) + PickupGuidOffset) = Pawn->GetCurrentWeapon() ? Pawn->GetCurrentWeapon()->GetItemEntryGuid() : FGuid();

	static auto OnRep_PickupLocationDataFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.OnRep_PickupLocationData");
	Pickup->ProcessEvent(OnRep_PickupLocationDataFn);

	Pickup->Get<bool>(bPickedUpOffset) = true;

	static auto OnRep_bPickedUpFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPickup.OnRep_bPickedUp");
	Pickup->ProcessEvent(OnRep_bPickedUpFn);
}

UClass* AFortPlayerPawn::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/FortniteGame.FortPlayerPawn");
	return Class;
}
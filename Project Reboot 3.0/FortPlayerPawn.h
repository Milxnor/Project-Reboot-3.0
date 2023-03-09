#pragma once

#include "FortPawn.h"
#include "FortPickup.h"

struct PadHex100 { char pad[0x100]; };

using FZiplinePawnState = PadHex100;

enum class EFortCustomPartType : uint8_t // todo move
{
	Head = 0,
	Body = 1,
	Hat = 2,
	Backpack = 3,
	Charm = 4,
	Face = 5,
	NumTypes = 6,
	EFortCustomPartType_MAX = 7
};

class AFortPlayerPawn : public AFortPawn
{
public:
	void ServerChoosePart(EFortCustomPartType Part, class UObject* ChosenCharacterPart);

	static void ServerSendZiplineStateHook(AFortPlayerPawn* Pawn, FZiplinePawnState InZiplineState);
	static void ServerHandlePickupHook(AFortPlayerPawn* Pawn, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound);
	static void ServerHandlePickupInfoHook(AFortPlayerPawn* Pawn, AFortPickup* Pickup, __int64 Params);

	static UClass* StaticClass();
};
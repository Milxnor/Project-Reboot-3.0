#pragma once

#include "FortPawn.h"
#include "FortPickup.h"

struct PadHex100 { char pad[0x100]; };

using FZiplinePawnState = PadHex100;

class AFortPlayerPawn : public AFortPawn
{
public:
	static void ServerSendZiplineStateHook(AFortPlayerPawn* Pawn, FZiplinePawnState InZiplineState);
	static void ServerHandlePickupHook(AFortPlayerPawn* Pawn, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound);
};
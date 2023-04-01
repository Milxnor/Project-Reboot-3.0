#include "FortWeapon.h"
#include "FortPlayerPawn.h"

#include "reboot.h"
#include "FortPlayerController.h"

void AFortWeapon::ServerReleaseWeaponAbilityHook(UObject* Context, FFrame* Stack, void* Ret)
{
	// I don't know where to put this..
	
	auto Weapon = (AFortWeapon*)Context;
	auto Pawn = Cast<AFortPlayerPawn>(Weapon->GetOwner());

	// LOG_INFO(LogDev, "Owner: {}", Weapon->GetOwner() ? Weapon->GetOwner()->GetFullName() : "InvalidObject");

	if (!Pawn)
		return ServerReleaseWeaponAbilityOriginal(Context, Stack, Ret);

	auto Controller = Cast<AFortPlayerController>(Pawn->GetController());
	auto CurrentWeapon = Weapon; // Pawn->GetCurrentWeapon();
	auto WorldInventory = Controller ? Controller->GetWorldInventory() : nullptr;

	if (!WorldInventory || !CurrentWeapon)
		return ServerReleaseWeaponAbilityOriginal(Context, Stack, Ret);

	static auto AmmoCountOffset = CurrentWeapon->GetOffset("AmmoCount");
	auto AmmoCount = CurrentWeapon->Get<int>(AmmoCountOffset);

	WorldInventory->CorrectLoadedAmmo(CurrentWeapon->GetItemEntryGuid(), AmmoCount);

	return ServerReleaseWeaponAbilityOriginal(Context, Stack, Ret);
}

UClass* AFortWeapon::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortWeapon");
	return Class;
}
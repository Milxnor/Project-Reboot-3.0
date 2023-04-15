#include "FortWeapon.h"
#include "FortPlayerPawn.h"

#include "reboot.h"
#include "FortPlayerController.h"

void AFortWeapon::OnPlayImpactFXHook(AFortWeapon* Weapon, __int64 HitResult, uint8_t ImpactPhysicalSurface, UObject* SpawnedPSC)
{
	// grappler

	auto Pawn = Cast<AFortPawn>(Weapon->GetOwner());

	if (!Pawn)
		return OnPlayImpactFXOriginal(Weapon, HitResult, ImpactPhysicalSurface, SpawnedPSC);

	auto Controller = Cast<AFortPlayerController>(Pawn->GetController());
	auto CurrentWeapon = Pawn->GetCurrentWeapon();
	auto WorldInventory = Controller ? Controller->GetWorldInventory() : nullptr;

	if (!WorldInventory || !CurrentWeapon)
		return OnPlayImpactFXOriginal(Weapon, HitResult, ImpactPhysicalSurface, SpawnedPSC);

	auto AmmoCount = CurrentWeapon->GetAmmoCount();

	WorldInventory->CorrectLoadedAmmo(CurrentWeapon->GetItemEntryGuid(), AmmoCount);

	return OnPlayImpactFXOriginal(Weapon, HitResult, ImpactPhysicalSurface, SpawnedPSC);
}

void AFortWeapon::ServerReleaseWeaponAbilityHook(UObject* Context, FFrame* Stack, void* Ret)
{
	return ServerReleaseWeaponAbilityOriginal(Context, Stack, Ret);
}

UClass* AFortWeapon::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortWeapon");
	return Class;
}
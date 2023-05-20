#include "FortPawn.h"

#include "reboot.h"
#include "FortPlayerControllerAthena.h"

AFortWeapon* AFortPawn::EquipWeaponDefinition(UFortWeaponItemDefinition* WeaponData, const FGuid& ItemEntryGuid)
{
	static auto EquipWeaponDefinitionFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.EquipWeaponDefinition");

	FGuid TrackerGuid{};

	if (Fortnite_Version < 16)
	{
		struct { UObject* Def; FGuid Guid; AFortWeapon* Wep; } params{ WeaponData, ItemEntryGuid };
		this->ProcessEvent(EquipWeaponDefinitionFn, &params);
		return params.Wep;
	}
	else if (std::floor(Fortnite_Version) == 16)
	{
		struct { UObject* Def; FGuid Guid; FGuid TrackerGuid; AFortWeapon* Wep; } S16_params{ WeaponData, ItemEntryGuid, TrackerGuid };
		this->ProcessEvent(EquipWeaponDefinitionFn, &S16_params);
		return S16_params.Wep;
	}
	else
	{
		struct { UObject* Def; FGuid Guid; FGuid TrackerGuid; bool bDisableEquipAnimation; AFortWeapon* Wep; } S17_params{ WeaponData, ItemEntryGuid, TrackerGuid, false };
		this->ProcessEvent(EquipWeaponDefinitionFn, &S17_params);
		return S17_params.Wep;
	}

	return nullptr;
}

bool AFortPawn::PickUpActor(AActor* PickupTarget, UFortDecoItemDefinition* PlacementDecoItemDefinition)
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.PickUpActor");
	struct { AActor* PickupTarget; UFortDecoItemDefinition* PlacementDecoItemDefinition; bool ReturnValue; } AFortPawn_PickUpActor_Params{ PickupTarget, PlacementDecoItemDefinition };
	this->ProcessEvent(fn, &AFortPawn_PickUpActor_Params);

	return AFortPawn_PickUpActor_Params.ReturnValue;
}

void AFortPawn::OnRep_IsDBNO()
{
	static auto OnRep_IsDBNOFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.OnRep_IsDBNO");
	this->ProcessEvent(OnRep_IsDBNOFn);
}

float AFortPawn::GetShield()
{
	float Shield = 0;
	static auto GetShieldFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.GetShield");

	if (GetShieldFn)
		this->ProcessEvent(GetShieldFn, &Shield);
	
	return Shield;
}

float AFortPawn::GetHealth()
{
	float Health = 0;
	static auto GetHealthFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.GetHealth");

	if (GetHealthFn)
		this->ProcessEvent(GetHealthFn, &Health);

	return Health;
}

void AFortPawn::SetHealth(float NewHealth)
{
	static auto SetHealthFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.SetHealth");

	if (SetHealthFn)
		this->ProcessEvent(SetHealthFn, &NewHealth);
}

void AFortPawn::SetMaxHealth(float NewHealthVal)
{
	static auto SetMaxHealthFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.SetMaxHealth");

	if (!SetMaxHealthFn)
		return;

	this->ProcessEvent(SetMaxHealthFn, &NewHealthVal);
}

void AFortPawn::SetShield(float NewShield)
{
	static auto SetShieldFn = FindObject<UFunction>("/Script/FortniteGame.FortPawn.SetShield");

	if (SetShieldFn)
		this->ProcessEvent(SetShieldFn, &NewShield);
}

void AFortPawn::NetMulticast_Athena_BatchedDamageCuesHook(UObject* Context, FFrame* Stack, void* Ret)
{
	auto Pawn = (AFortPawn*)Context;
	auto Controller = Cast<AFortPlayerController>(Pawn->GetController());
	auto CurrentWeapon = Pawn->GetCurrentWeapon();
	auto WorldInventory = Controller ? Controller->GetWorldInventory() : nullptr;

	if (!WorldInventory || !CurrentWeapon)
		return NetMulticast_Athena_BatchedDamageCuesOriginal(Context, Stack, Ret);

	auto AmmoCount = CurrentWeapon->GetAmmoCount();

	WorldInventory->CorrectLoadedAmmo(CurrentWeapon->GetItemEntryGuid(), AmmoCount);

	return NetMulticast_Athena_BatchedDamageCuesOriginal(Context, Stack, Ret);
}

void AFortPawn::MovingEmoteStoppedHook(UObject* Context, FFrame* Stack, void* Ret)
{
	// LOG_INFO(LogDev, "MovingEmoteStoppedHook!");

	auto Pawn = (AFortPawn*)Context;

	static auto bMovingEmoteOffset = Pawn->GetOffset("bMovingEmote", false);

	if (bMovingEmoteOffset != -1)
	{
		static auto bMovingEmoteFieldMask = GetFieldMask(Pawn->GetProperty("bMovingEmote"));
		Pawn->SetBitfieldValue(bMovingEmoteOffset, bMovingEmoteFieldMask, false);
	}

	static auto bMovingEmoteForwardOnlyOffset = Pawn->GetOffset("bMovingEmoteForwardOnly", false);

	if (bMovingEmoteForwardOnlyOffset != -1)
	{
		static auto bMovingEmoteForwardOnlyFieldMask = GetFieldMask(Pawn->GetProperty("bMovingEmoteForwardOnly"));
		Pawn->SetBitfieldValue(bMovingEmoteOffset, bMovingEmoteForwardOnlyFieldMask, false);
	}

	return MovingEmoteStoppedOriginal(Context, Stack, Ret);
}

UClass* AFortPawn::StaticClass()
{
	static auto Class = FindObject<UClass>("/Script/FortniteGame.FortPawn");
	return Class;
}
#pragma once

#include "Actor.h"
#include "reboot.h" // we want to prevent this but im to lazy to make cpp file
#include "PlayerController.h"

#include "GameplayTagContainer.h"

class ABuildingActor : public AActor
{
public:
	void InitializeBuildingActor(UObject* Controller, ABuildingActor* BuildingOwner, bool bUsePlayerBuildAnimations, UObject* ReplacedBuilding = nullptr)
	{
		struct {
			UObject* BuildingOwner; // ABuildingActor
			UObject* SpawningController;
			bool bUsePlayerBuildAnimations; // I think this is not on some versions
			UObject* ReplacedBuilding; // this also not on like below 18.00
		} IBAParams{ BuildingOwner, Controller, bUsePlayerBuildAnimations, ReplacedBuilding };

		static auto fn = FindObject<UFunction>("/Script/FortniteGame.BuildingActor.InitializeKismetSpawnedBuildingActor");
		this->ProcessEvent(fn, &IBAParams);
	}

	float GetMaxHealth()
	{
		float MaxHealth = 0;
		static auto fn = FindObject<UFunction>("/Script/FortniteGame.BuildingActor.GetMaxHealth");
		this->ProcessEvent(fn, &MaxHealth);
		return MaxHealth;
	}

	static inline void (*OnDamageServerOriginal)(ABuildingActor* BuildingActor, float Damage, FGameplayTagContainer DamageTags,
		FVector Momentum, /* FHitResult */ __int64  HitInfo, APlayerController* InstigatedBy, AActor* DamageCauser,
		/* FGameplayEffectContextHandle */ __int64 EffectContext);

	static void OnDamageServerHook(ABuildingActor* BuildingActor, float Damage, FGameplayTagContainer DamageTags,
		FVector Momentum, /* FHitResult */ __int64  HitInfo, APlayerController* InstigatedBy, AActor* DamageCauser,
		/* FGameplayEffectContextHandle */ __int64 EffectContext);

	static UClass* StaticClass();
};
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

	bool IsDestroyed()
	{
		static auto bDestroyedOffset = GetOffset("bDestroyed");
		static auto bDestroyedFieldMask = GetFieldMask(GetProperty("bDestroyed"));
		return ReadBitfieldValue(bDestroyedOffset, bDestroyedFieldMask);
	}

	void SilentDie()
	{
		static auto SilentDieFn = FindObject<UFunction>(L"/Script/FortniteGame.BuildingActor.SilentDie");
		bool bPropagateSilentDeath = false; // idfk
		this->ProcessEvent(SilentDieFn, &bPropagateSilentDeath);
	}

	float GetMaxHealth()
	{
		float MaxHealth = 0;
		static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.BuildingActor.GetMaxHealth");
		this->ProcessEvent(fn, &MaxHealth);
		return MaxHealth;
	}

	float GetHealthPercent() // aka GetHealth() / GetMaxHealth()
	{
		float HealthPercent = 0;
		static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.BuildingActor.GetHealthPercent");
		this->ProcessEvent(fn, &HealthPercent);
		return HealthPercent;
	}

	float GetHealth()
	{
		float Health = 0;
		static auto fn = FindObject<UFunction>("/Script/FortniteGame.BuildingActor.GetHealth");
		this->ProcessEvent(fn, &Health);
		return Health;
	}

	void SetTeam(unsigned char InTeam)
	{
		static auto fn = nullptr; // FindObject<UFunction>(L"/Script/FortniteGame.BuildingActor.SetTeam");

		if (!fn)
		{
			static auto TeamOffset = GetOffset("Team");
			Get<uint8_t>(TeamOffset) = InTeam;

			static auto TeamIndexOffset = GetOffset("TeamIndex", false);

			if (TeamIndexOffset != -1)
				Get<uint8_t>(TeamIndexOffset) = InTeam;
		}
		else
		{
			this->ProcessEvent(fn, &InTeam);
		}
	}

	bool IsPlayerBuildable()
	{
		static auto bIsPlayerBuildableOffset = GetOffset("bIsPlayerBuildable");
		static auto bIsPlayerBuildableFieldMask = GetFieldMask(GetProperty("bIsPlayerBuildable"));
		return ReadBitfieldValue(bIsPlayerBuildableOffset, bIsPlayerBuildableFieldMask);
	}

	static inline void (*OnDamageServerOriginal)(ABuildingActor* BuildingActor, float Damage, FGameplayTagContainer DamageTags,
		FVector Momentum, /* FHitResult */ __int64  HitInfo, APlayerController* InstigatedBy, AActor* DamageCauser,
		/* FGameplayEffectContextHandle */ __int64 EffectContext);

	static void OnDamageServerHook(ABuildingActor* BuildingActor, float Damage, FGameplayTagContainer DamageTags,
		FVector Momentum, /* FHitResult */ __int64  HitInfo, APlayerController* InstigatedBy, AActor* DamageCauser,
		/* FGameplayEffectContextHandle */ __int64 EffectContext);

	static UClass* StaticClass();
};
#pragma once

#include "Object.h"
#include "Array.h"
#include "FortAbilitySet.h"
#include "SoftObjectPtr.h"
#include "FortPlayerPawnAthena.h"
#include "GameplayTagContainer.h"
#include "BuildingActor.h"
#include "FortPlayerPawnAthena.h"
#include "GameplayAbilityTypes.h"

struct FGameplayTagRequirements
{
	FGameplayTagContainer                       RequireTags;                                              // 0x0000(0x0020) (Edit, BlueprintVisible, NativeAccessSpecifierPublic)
	FGameplayTagContainer                       IgnoreTags;                                               // 0x0020(0x0020) (Edit, BlueprintVisible, NativeAccessSpecifierPublic)
};

enum class EFortDeliveryInfoBuildingActorSpecification : uint8_t
{
	All = 0,
	PlayerBuildable = 1,
	NonPlayerBuildable = 2,
	EFortDeliveryInfoBuildingActorSpecification_MAX = 3
};

struct FFortDeliveryInfoRequirementsFilter
{
	bool ShouldApplyToPawns()
	{
		static auto bApplyToPlayerPawnsOffset = FindOffsetStruct("/Script/FortniteGame.FortDeliveryInfoRequirementsFilter", "bApplyToPlayerPawns");
		static auto bApplyToPlayerPawnsFieldMask = GetFieldMask(FindPropertyStruct("/Script/FortniteGame.FortDeliveryInfoRequirementsFilter", "bApplyToPlayerPawns"));
		return ReadBitfield((PlaceholderBitfield*)(__int64(this) + bApplyToPlayerPawnsOffset), bApplyToPlayerPawnsFieldMask);
	}

	bool ShouldApplyToBuildingActors()
	{
		static auto bApplyToBuildingActorsOffset = FindOffsetStruct("/Script/FortniteGame.FortDeliveryInfoRequirementsFilter", "bApplyToBuildingActors");
		static auto bApplyToBuildingActorsFieldMask = GetFieldMask(FindPropertyStruct("/Script/FortniteGame.FortDeliveryInfoRequirementsFilter", "bApplyToBuildingActors"));
		return ReadBitfield((PlaceholderBitfield*)(__int64(this) + bApplyToBuildingActorsOffset), bApplyToBuildingActorsFieldMask);
	}

	bool ShouldConsiderTeam()
	{
		static auto bConsiderTeamOffset = FindOffsetStruct("/Script/FortniteGame.FortDeliveryInfoRequirementsFilter", "bConsiderTeam");
		static auto bConsiderTeamFieldMask = GetFieldMask(FindPropertyStruct("/Script/FortniteGame.FortDeliveryInfoRequirementsFilter", "bConsiderTeam"));
		return ReadBitfield((PlaceholderBitfield*)(__int64(this) + bConsiderTeamOffset), bConsiderTeamFieldMask);
	}

	FGameplayTagRequirements& GetTargetTagRequirements()
	{
		static auto TargetTagRequirementsOffset = FindOffsetStruct("/Script/FortniteGame.FortDeliveryInfoRequirementsFilter", "TargetTagRequirements");
		return *(FGameplayTagRequirements*)(__int64(this) + TargetTagRequirementsOffset);
	}

	EFortDeliveryInfoBuildingActorSpecification& GetBuildingActorSpecification()
	{
		static auto BuildingActorSpecificationOffset = FindOffsetStruct("/Script/FortniteGame.FortDeliveryInfoRequirementsFilter", "BuildingActorSpecification");
		return *(EFortDeliveryInfoBuildingActorSpecification*)(__int64(this) + BuildingActorSpecificationOffset);
	}

	bool DoesActorFollowsRequirements(AActor* Actor)
	{
		// TODO ADD TEAM CHECK! (We can use UFortKismetLibrary::GetActorTeam)

		if (auto BuildingActor = Cast<ABuildingActor>(Actor))
		{
			if (!ShouldApplyToBuildingActors())
				return false;

			//if (GetTargetTagRequirements().RequireTags.GameplayTags.Num() > 0 && GetTargetTagRequirements().) // idk todo

			if (GetBuildingActorSpecification() == EFortDeliveryInfoBuildingActorSpecification::PlayerBuildable && BuildingActor->IsPlayerBuildable())
				return true;

			if (GetBuildingActorSpecification() == EFortDeliveryInfoBuildingActorSpecification::NonPlayerBuildable && !BuildingActor->IsPlayerBuildable())
				return true;

			return GetBuildingActorSpecification() == EFortDeliveryInfoBuildingActorSpecification::All;
		}

		else if (auto Pawn = Cast<AFortPlayerPawnAthena>(Actor))
		{
			return ShouldApplyToPawns();
		}

		else if (auto PlayerState = Cast<AFortPlayerState>(Actor))
		{
			return ShouldApplyToPawns(); // scuffed
		}

		return false;
	}
};

struct FFortGameplayEffectDeliveryInfo
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.FortGameplayEffectDeliveryInfo");
		return Struct;
	}

	static int GetStructSize()
	{
		return GetStruct()->GetPropertiesSize();
	}

	FFortDeliveryInfoRequirementsFilter* GetDeliveryRequirements()
	{
		static auto DeliveryRequirementsOffset = FindOffsetStruct("/Script/FortniteGame.FortGameplayEffectDeliveryInfo", "DeliveryRequirements");
		return (FFortDeliveryInfoRequirementsFilter*)(__int64(this) + DeliveryRequirementsOffset);
	}

	TArray<FGameplayEffectApplicationInfo>& GetGameplayEffects()
	{
		static auto GameplayEffectsOffset = FindOffsetStruct("/Script/FortniteGame.FortGameplayEffectDeliveryInfo", "GameplayEffects");
		return *(TArray<FGameplayEffectApplicationInfo>*)(__int64(this) + GameplayEffectsOffset);
	}
};

struct FFortAbilitySetDeliveryInfo
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/FortniteGame.FortAbilitySetDeliveryInfo");
		return Struct;
	}

	static int GetStructSize()
	{
		return GetStruct()->GetPropertiesSize();
	}

	FFortDeliveryInfoRequirementsFilter* GetDeliveryRequirements()
	{
		static auto DeliveryRequirementsOffset = FindOffsetStruct("/Script/FortniteGame.FortAbilitySetDeliveryInfo", "DeliveryRequirements");
		return (FFortDeliveryInfoRequirementsFilter*)(__int64(this) + DeliveryRequirementsOffset);
	}

	TArray<TSoftObjectPtr<UFortAbilitySet>>& GetAbilitySets()
	{
		static auto AbilitySetsOffset = FindOffsetStruct("/Script/FortniteGame.FortAbilitySetDeliveryInfo", "AbilitySets");
		return *(TArray<TSoftObjectPtr<UFortAbilitySet>>*)(__int64(this) + AbilitySetsOffset);
	}
};

class UFortGameplayModifierItemDefinition : public UObject
{
public:
	TArray<FFortGameplayEffectDeliveryInfo>& GetPersistentGameplayEffects()
	{
		static auto PersistentGameplayEffectsOffset = GetOffset("PersistentGameplayEffects");
		return this->Get<TArray<FFortGameplayEffectDeliveryInfo>>(PersistentGameplayEffectsOffset);
	}

	TArray<FFortAbilitySetDeliveryInfo>& GetPersistentAbilitySets()
	{
		static auto PersistentAbilitySetsOffset = GetOffset("PersistentAbilitySets");
		return this->Get<TArray<FFortAbilitySetDeliveryInfo>>(PersistentAbilitySetsOffset);
	}

	void ApplyModifierToActor(AActor* Actor)
	{
		if (!Actor)
			return;

		// TODO Use the UAbilitySystemInterface or whatever

		UAbilitySystemComponent* AbilitySystemComponent = nullptr;

		if (auto BuildingActor = Cast<ABuildingActor>(Actor))
		{
			static auto AbilitySystemComponentOffset = BuildingActor->GetOffset("AbilitySystemComponent");
			AbilitySystemComponent = BuildingActor->Get<UAbilitySystemComponent*>(AbilitySystemComponentOffset);
		}

		else if (auto PlayerState = Cast<AFortPlayerState>(Actor))
		{
			AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
		}

		else if (auto Pawn = Cast<AFortPlayerPawnAthena>(Actor))
		{
			static auto AbilitySystemComponentOffset = Pawn->GetOffset("AbilitySystemComponent");
			AbilitySystemComponent = Pawn->Get<UAbilitySystemComponent*>(AbilitySystemComponentOffset);
		}

		if (!AbilitySystemComponent)
		{
			LOG_INFO(LogDev, "Unable to find ASC for {}", Actor->GetName());
			return;
		}

		for (int z = 0; z < this->GetPersistentAbilitySets().Num(); z++)
		{
			auto& AbilitySetDeliveryInfo = this->GetPersistentAbilitySets().at(z, FFortAbilitySetDeliveryInfo::GetStructSize());

			if (!AbilitySetDeliveryInfo.GetDeliveryRequirements()->DoesActorFollowsRequirements(Actor))
				continue;
				
			auto& CurrentAbilitySets = AbilitySetDeliveryInfo.GetAbilitySets();

			for (int j = 0; j < CurrentAbilitySets.Num(); j++)
			{
				auto& CurrentAbilitySetSoft = CurrentAbilitySets.at(j);
				auto CurrentAbilitySet = CurrentAbilitySetSoft.Get(UFortAbilitySet::StaticClass(), true);

				if (!CurrentAbilitySet->IsValidLowLevel())
					continue;

				CurrentAbilitySet->GiveToAbilitySystem(AbilitySystemComponent);
			}
		}

		for (int z = 0; z < this->GetPersistentGameplayEffects().Num(); z++)
		{
			auto& GameplayEffectDeliveryInfo = this->GetPersistentGameplayEffects().at(z, FFortGameplayEffectDeliveryInfo::GetStructSize());

			if (!GameplayEffectDeliveryInfo.GetDeliveryRequirements()->DoesActorFollowsRequirements(Actor))
				continue;

			auto& CurrentGameplayEffects = GameplayEffectDeliveryInfo.GetGameplayEffects();

			for (int j = 0; j < CurrentGameplayEffects.Num(); j++)
			{
				auto& CurrentGameplayEffectInfo = CurrentGameplayEffects.at(j);
				auto& CurrentGameplayEffectSoft = CurrentGameplayEffectInfo.GameplayEffect;
				static auto ClassClass = FindObject<UClass>("/Script/CoreUObject.Class");
				auto CurrentGameplayEffect = CurrentGameplayEffectSoft.Get(ClassClass, true);

				if (!CurrentGameplayEffect)
					continue;

				// LOG_INFO(LogDev, "Giving GameplayEffect {}", CurrentGameplayEffect->GetFullName());
				AbilitySystemComponent->ApplyGameplayEffectToSelf(CurrentGameplayEffect, CurrentGameplayEffectInfo.Level);
			}
		}
	}
};


struct FAthenaScoreData
{

};

struct FWinConditionScoreData
{
	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>("/Script/FortniteGame.WinConditionScoreData");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }

	FScalableFloat* GetGoalScore()
	{
		static auto GoalScoreOffset = FindOffsetStruct("/Script/FortniteGame.WinConditionScoreData", "GoalScore");
		return (FScalableFloat*)(__int64(this) + GoalScoreOffset);
	}

	FScalableFloat* GetBigScoreThreshold()
	{
		static auto BigScoreThresholdOffset = FindOffsetStruct("/Script/FortniteGame.WinConditionScoreData", "BigScoreThreshold");
		return (FScalableFloat*)(__int64(this) + BigScoreThresholdOffset);
	}

	TArray<FAthenaScoreData>& GetScoreDataList()
	{
		static auto ScoreDataListOffset = FindOffsetStruct("/Script/FortniteGame.WinConditionScoreData", "ScoreDataList");
		return *(TArray<FAthenaScoreData>*)(__int64(this) + ScoreDataListOffset);
	}
};

class UFortPlaylist : public UObject
{
public:
	TArray<TSoftObjectPtr<UFortGameplayModifierItemDefinition>>& GetModifierList()
	{
		static auto ModifierListOffset = this->GetOffset("ModifierList");
		return this->Get<TArray<TSoftObjectPtr<UFortGameplayModifierItemDefinition>>>(ModifierListOffset);
	}

	FWinConditionScoreData* GetScoringData()
	{
		static auto ScoringDataOffset = GetOffset("ScoringData");
		return GetPtr<FWinConditionScoreData>(ScoringDataOffset);
	}

	void ApplyModifiersToActor(AActor* Actor)
	{
		if (!Actor)
			return;

		static auto ModifierListOffset = this->GetOffset("ModifierList", false);

		if (ModifierListOffset == -1)
			return;

		auto& ModifierList = this->GetModifierList();

		static auto FortGameplayModifierItemDefinitionClass = FindObject<UClass>(L"/Script/FortniteGame.FortGameplayModifierItemDefinition");

		for (int i = 0; i < ModifierList.Num(); ++i)
		{
			auto& ModifierSoft = ModifierList.at(i);
			auto StrongModifier = ModifierSoft.Get(FortGameplayModifierItemDefinitionClass, true);

			if (!StrongModifier)
				continue;

			StrongModifier->ApplyModifierToActor(Actor);
		}
	}
};
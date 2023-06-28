#pragma once

#include <unordered_map>
#include <vector>
#include <random>
#include <map>
#include <numeric>

#include "Array.h"
#include "FortWorldItemDefinition.h"
#include "SoftObjectPtr.h"
#include "FortItem.h"

struct FFortLootPackageData
{
public:
	FName& GetLootPackageID()
	{
		static auto LootPackageIDOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "LootPackageID");
		return *(FName*)(__int64(this) + LootPackageIDOffset);
	}

	float& GetWeight()
	{
		static auto WeightOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "Weight");
		return *(float*)(__int64(this) + WeightOffset);
	}

	FString& GetLootPackageCall()
	{
		static auto LootPackageCallOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "LootPackageCall");
		return *(FString*)(__int64(this) + LootPackageCallOffset);
	}

	TSoftObjectPtr<UFortItemDefinition>& GetItemDefinition()
	{
		static auto ItemDefinitionOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "ItemDefinition");
		return *(TSoftObjectPtr<UFortItemDefinition>*)(__int64(this) + ItemDefinitionOffset);
	}

	int& GetCount()
	{
		static auto CountOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "Count");
		return *(int*)(__int64(this) + CountOffset);
	}

	int& GetLootPackageCategory()
	{
		static auto LootPackageCategoryOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "LootPackageCategory");
		return *(int*)(__int64(this) + LootPackageCategoryOffset);
	}

	int& GetMinWorldLevel()
	{
		static auto MinWorldLevelOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "MinWorldLevel");
		return *(int*)(__int64(this) + MinWorldLevelOffset);
	}

	int& GetMaxWorldLevel()
	{
		static auto MaxWorldLevelOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "MaxWorldLevel");
		return *(int*)(__int64(this) + MaxWorldLevelOffset);
	}

	FString& GetAnnotation()
	{
		static auto AnnotationOffset = FindOffsetStruct("/Script/FortniteGame.FortLootPackageData", "Annotation");
		return *(FString*)(__int64(this) + AnnotationOffset);
	}
};

struct FFortLootTierData
{
public:
	float& GetNumLootPackageDrops()
	{
		static auto NumLootPackageDropsOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "NumLootPackageDrops");
		return *(float*)(__int64(this) + NumLootPackageDropsOffset);
	}

	FName& GetTierGroup()
	{
		static auto TierGroupOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "TierGroup");
		return *(FName*)(__int64(this) + TierGroupOffset);
	}

	int& GetLootTier()
	{
		static auto LootTierOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "LootTier");
		return *(int*)(__int64(this) + LootTierOffset);
	}

	float& GetWeight()
	{
		static auto WeightOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "Weight");
		return *(float*)(__int64(this) + WeightOffset);
	}

	FName& GetLootPackage()
	{
		static auto LootPackageOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "LootPackage");
		return *(FName*)(__int64(this) + LootPackageOffset);
	}

	TArray<int>& GetLootPackageCategoryWeightArray()
	{
		static auto LootPackageCategoryWeightArrayOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "LootPackageCategoryWeightArray");
		return *(TArray<int>*)(__int64(this) + LootPackageCategoryWeightArrayOffset);
	}

	TArray<int>& GetLootPackageCategoryMinArray()
	{
		static auto LootPackageCategoryMinArrayOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "LootPackageCategoryMinArray");
		return *(TArray<int>*)(__int64(this) + LootPackageCategoryMinArrayOffset);
	}

	TArray<int>& GetLootPackageCategoryMaxArray()
	{
		static auto LootPackageCategoryMaxArrayOffset = FindOffsetStruct("/Script/FortniteGame.FortLootTierData", "LootPackageCategoryMaxArray");
		return *(TArray<int>*)(__int64(this) + LootPackageCategoryMaxArrayOffset);
	}
};

struct LootDrop
{
	FFortItemEntry* ItemEntry;

	FFortItemEntry* operator->() {
		return ItemEntry;
	}

	~LootDrop()
	{

	}
};

static inline float RandomFloatForLoot(float AllWeightsSum)
{
	return (rand() * 0.000030518509f) * AllWeightsSum;
}

template <typename KeyType, typename ValueType>
FORCEINLINE static ValueType PickWeightedElement(const std::map<KeyType, ValueType>& Elements,
	std::function<float(ValueType)> GetWeightFn,
	std::function<float(float)> RandomFloatGenerator = RandomFloatForLoot,
	float TotalWeightParam = -1, bool bCheckIfWeightIsZero = false, int RandMultiplier = 1, KeyType* OutName = nullptr, bool bPrint = false, bool bKeepGoingUntilWeGetValue = false)
{
	float TotalWeight = TotalWeightParam;

	if (TotalWeight == -1)
	{
		TotalWeight = std::accumulate(Elements.begin(), Elements.end(), 0.0f, [&](float acc, const std::pair<KeyType, ValueType>& p) {
			auto Weight = GetWeightFn(p.second);

			if (bPrint)
			{
				// if (Weight != 0)
				{
					LOG_INFO(LogLoot, "Adding weight {}", Weight);
				}
			}

			return acc + Weight;
			});
	}

	float RandomNumber = // UKismetMathLibrary::RandomFloatInRange(0, TotalWeight);
		RandMultiplier * RandomFloatGenerator(TotalWeight);

	if (bPrint)
	{
		LOG_INFO(LogLoot, "RandomNumber: {} TotalWeight: {} Elements.size(): {}", RandomNumber, TotalWeight, Elements.size());
	}

	for (auto& Element : Elements)
	{
		float Weight = GetWeightFn(Element.second);

		if (bCheckIfWeightIsZero && Weight == 0)
			continue;

		if (RandomNumber <= Weight)
		{
			if (OutName)
				*OutName = Element.first;

			return Element.second;
		}

		RandomNumber -= Weight;
	}

	if (bKeepGoingUntilWeGetValue)
		return PickWeightedElement<KeyType, ValueType>(Elements, GetWeightFn, RandomFloatGenerator, TotalWeightParam, bCheckIfWeightIsZero, RandMultiplier, OutName, bPrint, bKeepGoingUntilWeGetValue);

	return ValueType();
}


template <typename KeyType, typename ValueType>
FORCEINLINE static ValueType PickWeightedElement(const std::unordered_map<KeyType, ValueType>& Elements,
	std::function<float(ValueType)> GetWeightFn,
	std::function<float(float)> RandomFloatGenerator = RandomFloatForLoot,
	float TotalWeightParam = -1, bool bCheckIfWeightIsZero = false, int RandMultiplier = 1, KeyType* OutName = nullptr, bool bPrint = false, bool bKeepGoingUntilWeGetValue = false)
{
	float TotalWeight = TotalWeightParam;

	if (TotalWeight == -1)
	{
		TotalWeight = std::accumulate(Elements.begin(), Elements.end(), 0.0f, [&](float acc, const std::pair<KeyType, ValueType>& p) {
			auto Weight = GetWeightFn(p.second);

			if (bPrint)
			{
				// if (Weight != 0)
				{
					LOG_INFO(LogLoot, "Adding weight {}", Weight);
				}
			}

			return acc + Weight;
			});
	}

	float RandomNumber = // UKismetMathLibrary::RandomFloatInRange(0, TotalWeight);
		RandMultiplier * RandomFloatGenerator(TotalWeight);

	if (bPrint)
	{
		LOG_INFO(LogLoot, "RandomNumber: {} TotalWeight: {} Elements.size(): {}", RandomNumber, TotalWeight, Elements.size());
	}

	for (auto& Element : Elements)
	{
		float Weight = GetWeightFn(Element.second);

		if (bCheckIfWeightIsZero && Weight == 0)
			continue;

		if (RandomNumber <= Weight)
		{
			if (OutName)
				*OutName = Element.first;

			return Element.second;
		}

		RandomNumber -= Weight;
	}

	if (bKeepGoingUntilWeGetValue)
		return PickWeightedElement<KeyType, ValueType>(Elements, GetWeightFn, RandomFloatGenerator, TotalWeightParam, bCheckIfWeightIsZero, RandMultiplier, OutName, bPrint, bKeepGoingUntilWeGetValue);

	return ValueType();
}

std::vector<LootDrop> PickLootDrops(FName TierGroupName, int WorldLevel, int ForcedLootTier = -1, bool bPrint = false, int recursive = 0, bool bCombineDrops = true);
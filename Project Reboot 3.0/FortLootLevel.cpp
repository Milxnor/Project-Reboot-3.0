#include "FortLootLevel.h"
#include "FortWorldItemDefinition.h"

int UFortLootLevel::GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int WorldLevel)
{
	// OMG IM GONNA DIE

	// we should use GetRows but L

	auto DataTable = LootLevelData.DataTable;

	if (!DataTable)
		return 0;

	if (!LootLevelData.ColumnName.ComparisonIndex.Value)
		return 0;

	if (!LootLevelData.RowContents.ComparisonIndex.Value)
		return 0;

	std::vector<FFortLootLevelData*> OurLootLevelDatas;

	for (auto& LootLevelDataPair : LootLevelData.DataTable->GetRowMap<FFortLootLevelData>())
	{
		if (LootLevelDataPair.Second->Category != LootLevelData.RowContents)
			continue;

		OurLootLevelDatas.push_back(LootLevelDataPair.Second);
	}

	if (OurLootLevelDatas.size() > 0)
	{
		int PickedIndex = -1;
		int PickedLootLevel = 0;

		for (int i = 0; i < OurLootLevelDatas.size(); i++)
		{
			auto CurrentLootLevelData = OurLootLevelDatas.at(i);

			if (CurrentLootLevelData->LootLevel <= WorldLevel && CurrentLootLevelData->LootLevel > PickedLootLevel)
			{
				PickedLootLevel = CurrentLootLevelData->LootLevel;
				PickedIndex = i;
			}
		}

		if (PickedIndex != -1)
		{
			auto PickedLootLevelData = OurLootLevelDatas.at(PickedIndex);

			const auto PickedMinItemLevel = PickedLootLevelData->MinItemLevel;
			const auto PickedMaxItemLevel = PickedLootLevelData->MaxItemLevel;
			auto v15 = PickedMaxItemLevel - PickedMinItemLevel;

			if (v15 + 1 <= 0)
			{
				v15 = 0;
			}
			else
			{
				auto v16 = (int)(float)((float)((float)rand() * 0.000030518509) * (float)(v15 + 1));
				if (v16 <= v15)
					v15 = v16;
			}

			return v15 + PickedMinItemLevel;
		}
	}

	return 0;
}
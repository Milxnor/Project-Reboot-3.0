#pragma once

#include "DataTable.h"

class UFortLootLevel
{
public:
	static int GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int WorldLevel);
};
#pragma once

#include "inc.h"

struct FNameEntryId
{
	uint32 Value;
};

struct FName
{
	FNameEntryId ComparisonIndex;
	uint32 Number;

	std::string ToString();

	bool IsValid() { return ComparisonIndex.Value > 0; }
};
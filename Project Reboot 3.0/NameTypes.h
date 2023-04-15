#pragma once

#include "inc.h"

struct FNameEntryId
{
	uint32 Value;

	FNameEntryId() : Value(0) {}

	FNameEntryId(uint32 value) : Value(value) {}
};

struct FName
{
	FNameEntryId ComparisonIndex;
	uint32 Number;

	std::string ToString() const;
	std::string ToString();

	FName() : ComparisonIndex(0), Number(0) {}

	FName(uint32 Value) : ComparisonIndex(Value), Number(0) {}

	bool IsValid() { return ComparisonIndex.Value > 0; }

	bool operator==(FName other)
	{
		return ComparisonIndex.Value == other.ComparisonIndex.Value;
	}
};
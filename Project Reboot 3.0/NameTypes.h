#pragma once

#include "inc.h"
#include "log.h"

struct FNameEntryId
{
	uint32 Value; // well depends on version if its int32 or uint32 i think

	FNameEntryId() : Value(0) {}

	FNameEntryId(uint32 value) : Value(value) {}

	bool operator<(FNameEntryId Rhs) const { return Value < Rhs.Value; }
	bool operator>(FNameEntryId Rhs) const { return Rhs.Value < Value; }
	bool operator==(FNameEntryId Rhs) const { return Value == Rhs.Value; }
	bool operator!=(FNameEntryId Rhs) const { return Value != Rhs.Value; }
};

#define WITH_CASE_PRESERVING_NAME 1 // ??

struct FName
{
	FNameEntryId ComparisonIndex;
	uint32 Number;

	FORCEINLINE int32 GetNumber() const
	{
		return Number;
	}

	FORCEINLINE FNameEntryId GetComparisonIndexFast() const
	{
		return ComparisonIndex;
	}

	std::string ToString() const;
	std::string ToString();

	FName() : ComparisonIndex(0), Number(0) {}

	FName(uint32 Value) : ComparisonIndex(Value), Number(0) {}

	bool IsValid() { return ComparisonIndex.Value > 0; } // for real

	FORCEINLINE bool operator==(const FName& Other) const // HMM??
	{
#if WITH_CASE_PRESERVING_NAME
		return GetComparisonIndexFast() == Other.GetComparisonIndexFast() && GetNumber() == Other.GetNumber();
#else
		// static_assert(sizeof(CompositeComparisonValue) == sizeof(*this), "ComparisonValue does not cover the entire FName state");
		// return CompositeComparisonValue == Other.CompositeComparisonValue;
#endif
	}

	int32 Compare(const FName& Other) const;

	/* FORCEINLINE bool operator<(const FName& Other) const
	{
		return Compare(Other) < 0;
	} */

	FORCEINLINE bool operator<(const FName& Rhs) const
	{
		auto res = this->ComparisonIndex == Rhs.ComparisonIndex ? /* (Number - Rhs.Number) < 0 */ Number < Rhs.Number : this->ComparisonIndex < Rhs.ComparisonIndex;
		// LOG_INFO(LogDev, "LHS ({} {}) RHS ({} {}) RESULT {}", this->ComparisonIndex.Value, this->Number, Rhs.ComparisonIndex.Value, Rhs.Number, res);
		return res;
		// return GetComparisonIndexFast() < Rhs.GetComparisonIndexFast() || (GetComparisonIndexFast() == Rhs.GetComparisonIndexFast() && GetNumber() < Rhs.GetNumber());

		// (Milxnor) BRO IDK

		if (GetComparisonIndexFast() == Rhs.GetComparisonIndexFast())
		{
			return (GetNumber() - Rhs.GetNumber()) < 0;
		}

		return GetComparisonIndexFast() < Rhs.GetComparisonIndexFast();
	}
};
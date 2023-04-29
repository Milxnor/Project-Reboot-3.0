#pragma once

#include "Object.h"
#include "CurveTable.h"
#include "UnrealString.h"

enum class EEvaluateCurveTableResult : uint8_t
{
	RowFound = 0,
	RowNotFound = 1,
	EEvaluateCurveTableResult_MAX = 2
};

class UDataTableFunctionLibrary : public UObject
{
public:
	static float EvaluateCurveTableRow(UCurveTable* CurveTable, FName RowName, float InXY,
		const FString& ContextString = FString(), EEvaluateCurveTableResult* OutResult = nullptr);

	static UClass* StaticClass();
};
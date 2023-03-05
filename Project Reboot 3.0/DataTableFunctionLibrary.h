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
	static void EvaluateCurveTableRow(UCurveTable* CurveTable, FName RowName, float InXY,
		FString ContextString, EEvaluateCurveTableResult* OutResult, float* OutXY);

	static UClass* StaticClass();
};
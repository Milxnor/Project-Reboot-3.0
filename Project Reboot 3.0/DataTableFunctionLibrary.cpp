#include "DataTableFunctionLibrary.h"

#include "reboot.h"

float UDataTableFunctionLibrary::EvaluateCurveTableRow(UCurveTable* CurveTable, FName RowName, float InXY,
	const FString& ContextString, EEvaluateCurveTableResult* OutResult)
{
	static auto fn = FindObject<UFunction>(L"/Script/Engine.DataTableFunctionLibrary.EvaluateCurveTableRow");

	float wtf{};
	EEvaluateCurveTableResult wtf1{};

	struct { UCurveTable* CurveTable; FName RowName; float InXY; EEvaluateCurveTableResult OutResult; float OutXY; FString ContextString; }
	UDataTableFunctionLibrary_EvaluateCurveTableRow_Params{CurveTable, RowName, InXY, wtf1, wtf, ContextString};

	static auto DefaultClass = StaticClass();
	DefaultClass->ProcessEvent(fn, &UDataTableFunctionLibrary_EvaluateCurveTableRow_Params);

	if (OutResult)
		*OutResult = UDataTableFunctionLibrary_EvaluateCurveTableRow_Params.OutResult;

	return UDataTableFunctionLibrary_EvaluateCurveTableRow_Params.OutXY;
}

UClass* UDataTableFunctionLibrary::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.DataTableFunctionLibrary");
	return Class;
}